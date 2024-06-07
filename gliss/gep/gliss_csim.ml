(*
 * This file is part of GLISS2 
 * Copyright (c) 2009-10, IRIT - UPS <casse@irit.fr>
 *
 * GLISS2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GLISS2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLISS2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *)

open Printf
open Irg

let text f = Templater.TEXT f
let coll f = Templater.COLL f
let bool f = Templater.BOOL f


(** Assign an index to each port.
	@return	Map of ports. *)
let assign_ports _ =
	fst (fold
		(fun name spec (map, i) ->
			match spec with
			| PORT(_, count, _, _) -> ((name, i)::map, i+count)
			| _ -> (map, i)	
		)
		([], 0)
	)


(** Build a templater symbol that output the given string.
	@param f	Function producing the string (takes () as parameter).
	@return		Templater symbol. *)
let out f =
	text (fun out -> output_string out (f ()))


(** Get the name of the component. *)
let comp_name _ =
	match get_string_let "component" with
	| None -> raise (Error (fun out -> fprintf out "'component' let is requiresd!"))
	| Some name -> name


(** Generate a text expression.
	@param info		Toc information.
	@param e		Expression to generate.
	@param out		Stream to output to. *)
let gen_string info e out =
	info.Toc.out <- out;
	let stats = Toc.prepare_stat info (Disasm.gen info UNDEF e) in
	Toc.declare_temps info;
	Toc.gen_stat info stats;
	Toc.cleanup_temps info


(** Generate code.
	@param info		Toc information.
	@param s		Statement to generate.
	@param out		Stream to output to. *)
let gen_code info s out =
	info.Toc.out <- out;
	let stats = Toc.prepare_stat info s in
	Toc.declare_temps info;
	Toc.gen_stat info stats;
	Toc.cleanup_temps info


(** Get an attribute as an integer.
	@param name		Attribute name.
	@param atts		Attribute list. *)
let get_int_att name atts =
	match get_attr name atts with
	| Some (ATTR_EXPR (_, e)) ->
		(try
			(match Sem.eval_const e with
			| CARD_CONST x -> Some x
			| _ -> None)
		with PreError f -> None)
	| _ -> None


(** Build collection symbol for registers. *)
let get_registers info f dict =

	let make name count typ atts dict =

		let offset out =
			fprintf out "0x%lx"
				(match get_int_att "offset" atts with
				| None -> pre_error "constant offset is required for registers"
				| Some x -> x) in

		let stride out =
			fprintf out "%ld"
				(match get_int_att "stride" atts with
				| None -> Int32.of_int ((Sem.get_type_length typ) / 8)
				| Some x -> x) in

		let ctype out =
			output_string out
				(match typ with
				| BOOL | INT _ | CARD _ | RANGE _ | ENUM _ ->
					"CSIM_INT"
				| x when x = ieee754_32 ->
					"CSIM_FLOAT32"
				| x when x = ieee754_64 ->
					"CSIM_FLOAT64"
				| _ ->
					failwith "Unsupported ctype.") in

		let label out =
			match get_attr "label" atts with
			| None ->
				fprintf out "snprintf(__buffer, size, \"%s%%d_%s\", inst->number);" (comp_name ()) name
			| Some (ATTR_EXPR (_, e)) ->
				if (Sem.get_type_expr e) <> STRING
				then pre_error "label must be of type string!"
				else gen_string info e out
			| _ ->
				pre_error "label must be a string expression!" in

		let display out =
			fprintf out "snprintf(__buffer, size, \"%%d\", __inst->%s);" name in

		let is_read_only out =
			false in
			(*match get_int_att "read_only" atts with
			| None -> pre_error "read_only must evaluate to 0 or to 1"
			| Some x -> x = Int32.zero in*)

		let is_write_only out =
			false in
			(*match get_int_att "write_only" atts with
			| None -> pre_error "write_only must evaluate to 0 or to 1"
			| Some x -> x = Int32.zero in*)
	
		("count", out (fun _ -> sprintf "%d" count)) ::
		("ctype", text ctype) ::
		("display", text display) ::
		("is_read_only", bool is_read_only) ::
		("is_write_only", bool is_write_only) ::
		("label", text label) ::
		("name", out (fun _ -> name)) ::
		("NAME", out (fun _ -> name)) ::
		("multiple", Templater.BOOL (fun _ -> count > 1)) ::
		("offset", text offset) ::
		("on_write", Templater.TEXT (fun out ->
			let action = Irg.attr_stat "on_write" atts Irg.NOP in
			let info = Toc.info () in
			info.Toc.out <- out;
			Toc.gen_stat info (Toc.prepare_stat info action)))::
		("size", out (fun _ -> sprintf "%d" ((Sem.get_type_length typ)/8))) ::
		("stride",  text stride) ::
		("type", out (fun _ -> Toc.type_to_string (Toc.convert_type typ))) ::
		dict in
	
	Irg.iter
		(fun name spec ->
			match spec with
			| Irg.REG(_, count, typ, atts) -> f (make name count typ atts dict)
			|  _ -> ())


(** Build collection symbol of ports. *)
let get_ports info pmap f dict =

	let make name count typ atts =

		let on_input out =
			match get_attr "on_input" atts with
			| None -> ()
			| Some (ATTR_STAT (_, s)) -> gen_code info s out
			| _ -> pre_error "on_input must be an attribute and define an instruction!" in

		let on_update out =
			match get_attr "on_update" atts with
			| None -> ()
			| Some (ATTR_STAT (_, s)) -> gen_code info s out
			| _ -> pre_error "on_update must be an attribute and define an instruction!" in

		let rec get_indexes i f dict =
			if i < count then begin
				f (
					("index", text (fun out -> fprintf out "%d" i)) ::
					dict
				);
				get_indexes (i + 1) f dict
			end in

		("base", text (fun out -> fprintf out "%d" (List.assoc name pmap))) ::
		("count", text (fun out -> fprintf out "%d" count)) ::
		("ctype", out (fun _ -> Toc.type_to_string (Toc.convert_type typ))) ::
		("indexes", coll (get_indexes 0)) ::
		("multiple", bool (fun _ -> count > 1)) ::
		("name", text (asis name)) ::
		("on_input", text on_input) ::
		("on_update", text on_update) ::
		("type", text (asis "CSIM_DIGITAL")) ::
		dict in

	Irg.iter
		(fun name spec ->
			match spec with
			| PORT (name, count, typ, atts) -> f (make name count typ atts)
			| _ -> ())


let get_events info f dict = 
	let make name atts dict = 

		let on_update out = 
			match get_attr "on_update" atts with
			| None -> ()
			| Some (ATTR_STAT (_,s)) -> gen_code info s out
			| _ -> pre_error "on_update must be an attribute and define an instruction!" in

		let on_trigger out =
			match get_attr "on_trigger" atts with 
			| None -> ()
			| Some (ATTR_STAT (_,s)) -> gen_code info s out
			| _ -> pre_error "on_update must be an attribute and define an instruction!" in

				("name", text (asis name)) ::
				("on_update", text on_update) ::
				("on_trigger", text on_trigger) ::
				dict in

				Irg.iter
					(fun name spec ->
						match spec with
						| EVENT (name, atts) -> f (make name atts dict)
						| _ -> ()) 

			(** Build the top-level dictionary. *)
let make_top_dict comp info =
	let pmap = assign_ports () in

	let register_count out =
		let cnt = fold (fun name spec cnt ->
			match spec with
			| REG _	-> cnt+1
			| _ 	-> cnt) 0 in
		fprintf out "%d" cnt in

	let port_count out =
		let cnt = fold (fun name spec cnt ->
			match spec with
			| PORT (_, count, _, _)	-> cnt+count
			| _ 		-> cnt) 0 in
		fprintf out "%d" cnt in

	let arch =
		match get_string_let "arch" with
		| None -> "NoArch"
		| Some name -> name in

	let io_comp = 
		match get_int_let "io_comp" with 
		| Some 1 -> true
		| _ -> false in

	[
		("comp", out (fun _ -> comp));
		("COMP", out (fun _ -> String.uppercase_ascii comp));
		("arch", out (fun _ -> arch));
		("ARCH", out (fun _ -> String.uppercase_ascii arch));
		("date", out (fun _ -> App.format_date (Unix.time ())));
		("port_count", text port_count);
		("ports", Templater.COLL (get_ports info pmap));
		("events", Templater.COLL (get_events info));
		("register_count", text register_count);
		("registers", Templater.COLL (get_registers info));
		("io_comp", bool (fun _ -> io_comp))
	]


(* main program *)
let _  =

	let process info =
		info.Toc.proc <- "";
		let comp = comp_name () in
		let dict = make_top_dict comp info in
		printf "Generating %s.h\n" comp;
		Templater.generate dict "csim.h" (comp ^ ".h");
		printf "Generating %s.c\n" comp;
		Templater.generate dict "csim.c" (comp ^ ".c") in
	
	App.run [] "GLISS generator of CSim component" process

