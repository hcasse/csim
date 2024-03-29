(*
 * $Id: app.ml,v 1.16 2009/11/26 09:01:16 casse Exp $
 * Copyright (c) 2009-10, IRIT - UPS <casse@irit.fr>
 *
 * This file is part of GLISS2.
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

module OrderedType =
struct
	type t = Toc.c_type
	let compare s1 s2 = if s1 = s2 then 0 else if s1 < s2 then (-1) else 1
end

module TypeSet = Set.Make(OrderedType);;

(** Gather information useful for the generation. *)
type maker_t = {
	mutable get_params: Iter.inst -> int -> string -> Irg.type_expr -> Templater.dict_t -> Templater.dict_t;
	mutable get_instruction: Iter.inst -> Templater.dict_t -> Templater.dict_t;
	mutable get_instruction_set: Iter.inst list -> Templater.dict_t -> Templater.dict_t;
	mutable get_register: Irg.spec -> Templater.dict_t -> Templater.dict_t
}



(** Build the given directory.
	@param path			Path of the directory.
	@raise Sys_error	If there is an error. *)
let rec makedir path =
	if not (Sys.file_exists path) then
		try
			(try
				let p = String.rindex path '/' in
				makedir (String.sub path 0 p)
			with Not_found -> ());
			Printf.printf "creating \"%s\"\n" path;
			Unix.mkdir path 0o740
		with Unix.Unix_error (code, _, _) ->
			raise (Sys_error (Printf.sprintf "cannot create \"%s\": %s" path (Unix.error_message code)))
	else
		let stats = Unix.stat path in
		if stats.Unix.st_kind <> Unix.S_DIR
		then raise (Sys_error (Printf.sprintf "cannot create directory \"%s\": file in the middle" path))



(* regular expressions *)
let lower_re = Str.regexp "gliss_"
let upper_re = Str.regexp "GLISS_"
let path_re = Str.regexp "gliss/"

(** Replace the "gliss" and "GLISS" words in the input file
	to create the output file.
	@param info		Generation information.
	@param in_file	Input file.
	@param out_file	Output file. *)
let replace_gliss info in_file out_file =
	let in_stream = open_in_bin in_file in
	let out_stream = open_out_bin out_file in
	let lower = info.Toc.proc ^ "_" in
	let upper = Config.uppercase lower in
	let rec trans _ =
		let line = input_line in_stream in
		output_string out_stream
			(Str.global_replace path_re (info.Toc.proc ^ "/")
			(Str.global_replace upper_re upper
			(Str.global_replace lower_re lower line)));
		output_char out_stream '\n';
		trans () in
	try
		trans ()
	with End_of_file ->
		close_in in_stream;
		close_out out_stream


(* Test if memory or register attributes contains ALIAS.
	@param attrs	Attributes to test.
	@return			True if it contains "alias", false else. *)
let rec contains_alias attrs =
	match attrs with
	  [] -> false
	| (Irg.ATTR_LOC ("alias", _))::_ -> true
	| _::tl -> contains_alias tl


(** Format date (in seconds) and return a stirng.
	@param date	Date to format.
	@return		Date formatted as a string. *)
let format_date date =
	let tm = Unix.localtime date in
	Printf.sprintf "%0d/%02d/%02d %02d:%02d:%02d"
		tm.Unix.tm_year tm.Unix.tm_mon tm.Unix.tm_mday
		tm.Unix.tm_hour tm.Unix.tm_min tm.Unix.tm_sec


(** Evaluate an expression in the context of the given speficiation.
	@param info		Current generation information.
	@param spec		Container specification.
	@param out		Current output.
	@param e		Expression to evaluate. *)
let eval_expr info spec out e =
	Irg.in_spec_context spec
		(fun _ -> 
			let (s, e) = Toc.prepare_expr info Irg.NOP e in
			Toc.declare_temps info;
			Toc.gen_stat info s;
			Toc.gen_expr info e true;
			output_string info.Toc.out "\n")


(** Generate the pre-statemet of an expression.
	@param info		Current generation information.
	@param spec		Container specification.
	@param out		Current output.
	@param e		Expression to evaluate. *)
let gen_pre_expr info spec out e =
	Irg.in_spec_context spec
		(fun _ -> 
			let (s, e) = Toc.prepare_expr info Irg.NOP e in
			Toc.declare_temps info;
			Toc.gen_stat info s;
			Toc.cleanup_temps info)


(** Generate the expression alone.
	@param info		Current generation information.
	@param spec		Container specification.
	@param out		Current output.
	@param e		Expression to evaluate. *)
let gen_expr info spec out e =
	Irg.in_spec_context spec
		(fun _ -> 
			Toc.gen_expr info e true;
			Toc.cleanup_temps info)


(** Evaluate a statement in the context of the given speficiation.
	@param info		Current generation information.
	@param spec		Container specification.
	@param out		Current output.
	@param s		Statement to evaluate. *)
let eval_stat info spec out s =
	Irg.in_spec_context spec
		(fun _ -> 
			let s = Toc.prepare_stat info s in
			Toc.declare_temps info;
			Toc.gen_stat info s;
			Toc.cleanup_temps info)


(** Evaluates an attribute by its name inside an instruction.
	@param spec		Current instruction.
	@param out		Output channel.
	@param arg		Argument: ID or ID:DEFAULT. *)
let eval_attr info spec out arg =
	let id, def =
		try
			let p = String.index arg ':' in
			(String.sub arg 0 p), (String.sub arg (p + 1) ((String.length arg) - p - 1))
		with Not_found -> arg, "" in
	try
		match Iter.get_attr spec id with
		| Iter.EXPR e -> eval_expr info spec out e
		| Iter.STAT s -> eval_stat info spec out s
	with Not_found ->
		output_string out def


(** Test if an attribute is defined.
	@param inst		Current instruction.
	@param id		Attribute identifier. *)
let defined_attr inst id =
	try
		ignore (Iter.get_attr inst id);
		true
	with Not_found -> false


(** Shortcut for templater text output from a string.
	f	Function to get string from. *)
let out f = Templater.TEXT (fun out -> output_string out (f ()))


(** Make a dictionary that supports attributes of a specification.
	In fact, add default identifiers that will resolve the symbols
	using the attributes named (prefixed by ".").
	@param info	Current generation information.
	@param spec	Specification to get attributes from.
	@param dict	Parent dictionnary.
	@return		New dictionnary. *)
let make_attr_dict info spec dict =
	
	let define dict att =
		match att with
		| Irg.ATTR_EXPR (n, e) ->
			(n, Templater.TEXT (fun out -> gen_expr info spec out e)) ::
			("pre-" ^ n, Templater.TEXT (fun out -> gen_pre_expr info spec out e)) ::
			dict
		| Irg.ATTR_STAT (n, s) ->
			(n, Templater.TEXT (fun out -> eval_stat info spec out s)) ::
			dict
		| _ -> dict in
	
	List.fold_left define dict (Irg.attrs_of spec)


(** Extended a dictionary with parameter information.
	@param name		Parameter name.
	@param typ		Parameter type.
	@param num		Parameter number (0 for first parameter).
	@param dict		Dictionary to extend.
	@return			Extended dictionary. *)
let make_param_dict name typ num dict =
	("PARAM",		out (fun _ -> name)) ::
	("INDEX",		out (fun _ -> string_of_int num)) ::
	("TYPE",		out (fun _ -> Toc.type_to_string (Toc.convert_type typ))) ::
	("PARAM_TYPE",	out (fun _ -> Config.uppercase (Toc.type_to_field (Toc.convert_type typ)))) ::
	("param_type", 	out (fun _ -> Toc.type_to_field (Toc.convert_type typ))) ::
	dict


(** Call f with each parameter of the instruction.
	@param maker	Current maker.
	@param f		Function to call.
	@param dict		Initial dictionary. *)
let get_params maker inst f dict =

	let get_type t =
		match t with
		  Irg.TYPE_EXPR t -> t
		| Irg.TYPE_ID n ->
			(match (Irg.get_symbol n) with
			  Irg.TYPE (_, t, _) -> t
			| _ -> Irg.NO_TYPE) in

	ignore (List.fold_left
		(fun (i: int) (n, t) ->
			let t = get_type t in
			(if t <> Irg.NO_TYPE then
				f (maker.get_params inst i n t
					(make_param_dict n t i dict)));
			i + 1)
		0
		(Iter.get_params inst))


(** Extended dictionary with definitions for an AND operation.
	@param name		Operation name.
	@param pars		Operation parameters.
	@param atts		Operation attributes.
	@param dict		Dictionary to extend.
	@return			Extended dictionary. *)
let make_op_dict name pars atts dict =
	let process f dict = 
		ignore (List.fold_left
			(fun i (n, t) ->
				let t = Sem.get_expr_from_type t in
				f (make_param_dict n t i dict);
				i + 1)
			0
			pars) in
	("IDENT", out (fun _ -> Config.uppercase name)) ::
	("ident", out (fun _ -> name)) ::
	("params", Templater.COLL process) ::
	("has_param", Templater.BOOL (fun _ -> (List.length pars) > 0)) ::
	("num_params", Templater.TEXT (fun out -> Printf.fprintf out "%d" (List.length pars))) ::
	dict


(** Default implementation to access an instruction from "instructions" list.
	Call the "get_instruction" from the maker with a dictionary augmanted
	with "IDENT", "ident", "ICODE", "params", "has_param", "num_params",
	"is_inst_branch", "attr" and "predecode".
	@param info		Current generation information (not set for instruction action).
	@param maker	Current maker structure.
	@param dict		Environment dictionnary.
	@param i		Current instruction. *)
let get_instruction info maker f dict _ i =
	let gen_predecode out =
		try
			let _ = Iter.get_attr i "predecode" in
			info.Toc.out <- out;
			Toc.set_inst info i;
			Toc.gen_action info "predecode"
		with Not_found -> () in

	f (maker.get_instruction  i
		(("ICODE", Templater.TEXT (fun out -> Printf.fprintf out "%d" (Iter.get_id i))) ::
		("is_inst_branch", Templater.BOOL (fun _ -> Iter.is_branch_instr i )) ::
		("attr", Templater.FUN (eval_attr info i)) ::
		("params", Templater.COLL (get_params maker i)) ::
		("predecode", Templater.TEXT gen_predecode)::
		(make_op_dict (Iter.get_name i) (Iter.get_params i) (Iter.get_attr i) dict)))


(** Get the nth first instructions defined by nb_inst
	Only if a instruction profile is loaded i.e : (Iter.instr_stats <> [])
*)
let get_ninstruction info maker f dict nb_inst cpt i =
	if (Iter.instr_stats = ref [])
	then (prerr_string "WARNING a profiling option is being used without a loaded '.profile'\n";cpt)
	else
		if (cpt < nb_inst)
		then let _ = get_instruction info maker f dict () i in cpt+1
		else cpt


exception BadCSize


(** Build a dictionary for an instruction set.
	@param maker	Current maker.
	@param f		Function to apply on the dictionnary.
	@param dict		Initial dictionnary.
	@param i_set	Instruction set information.
	@return			Dictionnary with instruction set variables. *)
let get_instruction_set maker f dict i_set =
	let min_size =
		List.fold_left
			(fun min inst ->
				let size = Iter.get_instruction_length inst
				in if size < min then size else min)
			1024 i_set
	in
	let max_size =
		List.fold_left
			(fun max inst ->
				let size = Iter.get_instruction_length inst
				in if size > max then size else max)
			0 i_set
	in
	let is_RISC =
		if min_size == max_size then
			(match min_size with
			| 8
			| 16
			| 32
			| 64 -> true
			| _ -> false
			)
		else
			false
	in
	let get_C_size n =
		match n with
		| _ when n > 0 && n <= 8 -> 8
		| _ when n > 8 && n <= 16 -> 16
		| _ when n > 16 && n <= 32 -> 32
		| _ when n > 32 && n <= 64 -> 64
		| _ -> raise BadCSize
	in
	let get_msb_mask n =
		try
			(match get_C_size min_size with
			| 8 -> "0x80"
			| 16 -> "0x8000"
			| 32 -> "0x80000000"
			| 64 -> "0x8000000000000000LL"
			| _ -> raise BadCSize)
		with
			BadCSize -> raise (Sys_error "template $(msb_mask_iset) should be used only with a RISC ISA")
	in
	let find_idx _ =
		let l_mem = List.map (fun x -> x = i_set) !Iter.multi_set in
		let rec aux l i =
			match l with
			| [] -> failwith "shouldn't happen (app.ml::get_instruction_set::find_idx)"
			| a::b -> if a then i else aux b (i + 1)
		in
		aux l_mem 0
	in f
	(maker.get_instruction_set i_set
		(("is_RISC_iset", Templater.BOOL (fun _ -> is_RISC)) ::
		("C_size_iset", Templater.TEXT (fun out -> Printf.fprintf out "%d" (try (get_C_size min_size) with BadCSize -> raise (Sys_error "template $(C_inst_size_iset) should be used only with a RISC ISA")))) ::
		(* return a mask for the most significant bit, size depends on the C size needed *)
		("msb_mask_iset", out (fun _ -> (get_msb_mask (min_size)))) ::
		(* iset select condition *)
		("select_iset", Templater.TEXT (fun out ->
			let info = Toc.info () in
			let spec_ = List.hd i_set in
			let select_attr =
				match Iter.get_attr spec_ "instruction_set_select" with
				| Iter.EXPR(e) -> e
				| _ -> failwith "(app.ml::get_instruction_set::$(iset_select)) attr instruction_set_select for op init must be an expr"
			in
			info.Toc.out <- out;
			info.Toc.inst <- spec_;
			info.Toc.iname <- "";
			(* stack params and attrs for the chosen instr *)
			match spec_ with
			| Irg.AND_OP(_, param_l, attr_l) ->
				Irg.param_stack param_l;
				Irg.attr_stack attr_l;
				(*Toc.gen_expr info select_attr true;*)
				Toc.gen_expr info (snd (Toc.prepare_expr info Irg.NOP select_attr)) true;
				Irg.param_unstack param_l;
				Irg.attr_unstack attr_l
			| _ -> failwith "(app.ml::get_instruction_set::$(iset_select)) shouldn't happen.")) ::
		(* index, 0 to n, as in !Iter.multi_set *)
		("idx", Templater.TEXT (fun out -> Printf.fprintf out "%d" (find_idx ()))) ::
		(* as described in nmp attr "instruction_set_name" *)
		("iset_name", Templater.TEXT (fun out ->
			let spec_ = List.hd i_set in
			let att = Irg.attr_expr "instruction_set_name" (Irg.attrs_of spec_) Irg.NONE in
			let name = match Irg.as_const_string att with
				| Some name -> name
				| None ->
					if (List.length !Iter.multi_set) > 1
					then Irg.error_spec spec_ (Irg.asis "no attribute 'instruction_set_name'")
					else Irg.get_proc_name () in
			output_string out name)) ::
		("min_size", Templater.TEXT (fun out -> Printf.fprintf out "%d" min_size)) ::
		dict))


(** Check if the attributes contains "pc" or "is_pc" attribute.
	Notice that "pc" is now deprecated.
	@param attrs	Attributes to look in.
	@return			True if one of these attributes is defined, false else. *)
let rec is_pc attrs =
	match attrs with
	| [] -> false
	| Irg.ATTR_EXPR ("pc", _) :: _
	| Irg.ATTR_EXPR ("is_pc", _) :: _	-> true
	| _ :: tl							-> is_pc tl

let is_float t =
	match t with
	| Irg.FLOAT _ -> true
	| _ -> false

let rec reg_format id size attrs =
	match attrs with
	| [] -> if size > 1 then id ^ "%d" else id
	| (Irg.ATTR_EXPR ("fmt", e))::_ ->
		(try Sem.to_string e
		with Irg.Error _ | Irg.PreError _ -> Toc.error "bad \"fmt\" attribute: should evaluate to string")
	| _ :: tl -> reg_format id size tl


(** Generate the code for accessing a register.
	@param name		Name of the register.
	@param size		Size of the register bank.
	@param typ		Type of the register.
	@param attrs	List of attributes.
	@param out		Output channel to use.
	@param make		Maker if the attribute is not available.
	@param attr		Name of the attribute. *)
let gen_reg_access name size typ attrs out attr make =
	let s = Sem.get_type_length typ in
	let v =
		if not (is_float typ)
		then if s <= 32 then "I" else "L"
		else if s <= 32 then "F" else "D" in
	let attrs =
		if Irg.attr_defined attr attrs then attrs else
		(Irg.ATTR_STAT (attr, make v))::attrs in
	let info =  Toc.info () in
	let spec = Irg.AND_OP ("instruction", [], attrs) in
	info.Toc.out <- out;
	info.Toc.inst <- spec;
	info.Toc.iname <- "";
	Irg.in_spec_context spec (fun _ -> Toc.gen_action info attr)


(** Make a canonic variable access.
	@param t	Type of variable.
	@param v	Name of the variable. *)
let make_canon_var t v =
	Irg.CONST (t, Irg.CANON(v))
	

(** Generate the setter of a register value for a debugger.
	@param name		Name of the register.
	@param size		Size of the register bank.
	@param typ		Type of the register.
	@param attrs	List of attributes.
	@param out		Output channel to use. *)
let gen_reg_setter name size typ attrs out =
	gen_reg_access name size typ attrs out "set"
		(fun v -> Irg.SET(
			Irg.LOC_REF (typ, name, (if size == 1 then Irg.NONE else make_canon_var typ "GLISS_IDX"), Irg.NONE, Irg.NONE),
			make_canon_var typ (Printf.sprintf "GLISS_%s" v)
		))


(** Generate the getter of a register value for a debugger.
	@param name		Name of the register.
	@param size		Size of the register bank.
	@param typ		Type of the register.
	@param attrs	List of attributes.
	@param out		Output channel to use. *)
let gen_reg_getter name size typ attrs out =
	gen_reg_access name size typ attrs out "get"
		(fun v -> Irg.CANON_STAT(
			Printf.sprintf "GLISS_GET_%s" v,
			[ if size == 1 then Irg.REF (typ, name) else Irg.ITEMOF (typ, name, make_canon_var typ "GLISS_IDX")]))


(** Get the label of a register bank.
	@param name		Name of the register banks.
	@param attrs	Attributes of the register bank.
	@return			Label of the register bank. *)
let get_label name attrs =
	let l = Irg.attr_expr "label" attrs Irg.NONE in
	if l = Irg.NONE then name else
	try Sem.to_string l
	with Irg.Error _ | Irg.PreError _ -> Toc.error "\"label\" attribute should be a string constant !"


(** Test if debugging information is available for a register.
	@param atts		Register attributes. *)
let is_debug atts = 
	if Irg.is_defined "gliss_debug_only"
	then Irg.attr_defined "debug" atts
	else not (contains_alias atts)


(** Extended the dictionary with definition for a register.
	@param info		Current generation information.
	@param spec		Register specification.
	@param dict		Dictionary to extend.
	@return			Extended dictionary. *)
let make_register_dict info spec dict =
	let (name, size, typ, atts) = 
		match spec with
		| Irg.REG (n, s, t, a) -> (n, s, t, a)
		| _ -> failwith "App.make_register_dict" in
	("aliased", Templater.BOOL (fun _ -> contains_alias atts)) ::
	("array", Templater.BOOL (fun _ -> size > 1)) ::
	("format", out (fun _ -> "\"" ^ (reg_format name size atts) ^ "\"")) ::
	("get", Templater.TEXT (fun out -> gen_reg_getter name size typ atts out)) ::
	("is_debug", Templater.BOOL (fun _ -> is_debug atts)) ::
	("is_float", Templater.BOOL (fun _ -> is_float typ)) ::
	("is_pc", Templater.BOOL (fun _ -> is_pc atts)) ::
	("label", out (fun _ -> get_label name atts)) ::
	("NAME", out (fun _ -> Config.uppercase name)) ::
	("name", out (fun _ -> name)) ::
	("printf_format", out (fun _ -> Toc.type_to_printf_format (Toc.convert_type typ))) ::
	("set", Templater.TEXT (fun out -> gen_reg_setter name size typ atts out)) ::
	("size", out (fun _ -> string_of_int size)) ::
	("type", out (fun _ -> Toc.type_to_string (Toc.convert_type typ))) ::
	("type_size", out (fun _ -> string_of_int (Sem.get_type_length typ))) ::
	(make_attr_dict info spec dict)


(** Test if the given specification is a register and call f with
	a dictionary extended with the register description.
	@param info		Current generation information.
	@param id		Unique identifier variable.
	@param f		Function to call.
	@param maker	Maker description.
	@param spec		Specification to scan. *)
let get_register info id f dict maker _ spec =
	match spec with
	| Irg.REG (name, size, t, atts) ->
		incr id;
		f (maker.get_register spec
			(("id", out (fun _ -> string_of_int !id)) ::
			(make_register_dict info spec dict)))
	| _ -> ()

let get_value f dict t =
	f (
		("name", out (fun _ -> Toc.type_to_field t)) ::
		("type", out (fun _ -> Toc.type_to_string t)) ::
		dict
	)

let get_param f dict t =
	f (
		("NAME", out (fun _ -> Config.uppercase (Toc.type_to_field t))) ::
		dict
	)


(** Build a dictionary with memory description.
	@param name	Memory name.
	@param size	Memory size.
	@param typ	Memory type.
	@param dict	Dictionary to extend.
	@param atts	Memory attributes.
	@return		Extended dictionary. *)
let make_memory_dict name size typ atts dict =
	("NAME", out (fun _ -> Config.uppercase name)) ::
	("name", out (fun _ -> name)) ::
	("aliased", Templater.BOOL (fun _ -> contains_alias atts)) ::
	dict


(** Scan the given specification. If it is an unaliased memory, run f
	with the dictionary improved with memory definitions.
	a solid memory (not aliased).
	@param f	Function to run.
	@param dict	Initial dictionary.
	@param key	Memory name.
	@param spec	Specification to scan. *)
let get_memory f dict key spec =
	match spec with
	 | Irg.MEM (name, size, Irg.CARD(8), atts)
	 (* you can find int(8) memories in some description (carcore, hcs12) *)
	 | Irg.MEM (name, size, Irg.INT(8), atts) ->
	  	f (make_memory_dict name size (Irg.INT(8)) atts dict)
	| _ -> ()


(** Manage the list of exceptions.
	@param f	Function to iterate on.
	@param dict	Current dictionary. *)
let list_exceptions f dict =

	let is_irq attrs =
		if Irg.attr_defined "is_irq" attrs then "1" else "0" in

	let gen_action attrs out =
		let info =  Toc.info () in
		info.Toc.out <- out;
		info.Toc.inst <- (Irg.AND_OP ("exception", [], attrs));
		info.Toc.iname <- "";
		Irg.attr_stack attrs;
		Toc.gen_action info "action";
		Irg.attr_unstack attrs in

	let gen exn attrs =
		f (
			("name", out (fun _ -> exn)) ::
			("is_irq", out (fun _ -> is_irq attrs)) ::
			("action", Templater.TEXT (gen_action attrs)) ::
			dict
		) in

	let rec process exn =
			(match Irg.get_symbol exn with
			| Irg.OR_OP (_, exns, _) -> List.iter process exns
			| Irg.AND_OP (_, _, attrs) -> gen exn attrs
			| _ -> Toc.error (Printf.sprintf "%s exception should an AND or an OR operation" exn)) in

		(match Irg.get_symbol "exceptions" with
		| Irg.UNDEF -> ()
		| Irg.OR_OP (_, exns,_) -> List.iter process exns
		| _ -> ())


let maker _ = {
	get_params = (fun _ _ _ _ dict -> dict);
	get_instruction = (fun _ dict -> dict);
	get_instruction_set = (fun _ dict -> dict);
	get_register = (fun _ dict -> dict)
}

let profiled_switch_size = ref 0


(** Traverse all debug registers (see documentation for more details).
	@param info		Current generation information.
	@param maker	Current maker.
	@param f		Function to call with each registers.
	@param dict		Current dictionary. *)
let debug_registers info maker f dict =
	let id = ref 0 in

	let process n r = get_register info id f dict maker n r in
	
	let process_only k r =
		match r with
		| Irg.REG(_, _, _, attrs) when Irg.attr_defined "debug" attrs -> process k r
		| _ -> () in

	if Irg.is_defined "gliss_debug_only"
	then Irg.StringHashtbl.iter process_only Irg.syms
	else Irg.StringHashtbl.iter process Irg.syms
	
	
(**	Build the basic environment for GEP generation.
	@param info		Generation environment.
	@param maker	Maker functions (as produced by maker constructor).
*)
let make_env info maker =
	let reg_id = ref 0 in

	let param_types =
		let collect_field set (name, t) =
			match t with
			  Irg.TYPE_EXPR t -> TypeSet.add (Toc.convert_type t) set
			| Irg.TYPE_ID n ->
				(match (Irg.get_symbol n) with
				  Irg.TYPE (_, t, _) -> TypeSet.add (Toc.convert_type t) set
				| _ -> set) in

		let collect_fields set params =
			List.fold_left collect_field set params in
		Iter.iter (fun set i -> collect_fields set (Iter.get_params i)) TypeSet.empty in

	("instructions", Templater.COLL (fun f dict -> Iter.iter (get_instruction info maker f dict) ())) ::
	("mapped_instructions", Templater.COLL (fun f dict -> Iter.iter_ext (get_instruction info maker f dict) () true)) ::
	("profiled_instructions", Templater.COLL (fun f dict -> 
	  let _ = Iter.iter_ext (get_ninstruction info maker f dict (!profiled_switch_size)) 0 true in () )) ::
	("instruction_sets", Templater.COLL (fun f dict -> List.iter (get_instruction_set maker f dict) !Iter.multi_set )) ::
	("registers", Templater.COLL (fun f dict -> reg_id := 0; Irg.iter (get_register info reg_id f dict maker))) ::
	("values", Templater.COLL (fun f dict -> TypeSet.iter (get_value f dict) param_types)) ::
	("params", Templater.COLL (fun f dict -> TypeSet.iter (get_param f dict) param_types)) ::
	("memories", Templater.COLL (fun f dict -> Irg.StringHashtbl.iter (get_memory f dict) Irg.syms)) ::
	("date", out (fun _ -> format_date (Unix.time ()))) ::
	("proc", out (fun _ -> info.Toc.proc)) ::
	("PROC", out (fun _ -> Config.uppercase info.Toc.proc)) ::
	("version", out (fun _ -> "GLISS V2.0 Copyright (c) 2009 IRIT - UPS")) ::
	("SOURCE_PATH", out (fun _ -> info.Toc.spath)) ::
	("INCLUDE_PATH", out (fun _ -> info.Toc.ipath)) ::
	("exceptions", Templater.COLL (fun f dict -> list_exceptions f dict)) ::
	[]


(* Activate a switch.
	@param name		Switch name.
	@param value	Value of the switch.
	@param dict		Dictionnary to activate in.
	@return			Result dictionnary. *)
let add_switch name value dict =
	(name, Templater.BOOL (fun _ -> value))::dict


(**
 * Load a NMP file and launch the given function on it
 * (and capture and display all exceptions).
 * @param file	File to process.
 * @param f		Function to work with definitions.
 * @param opti             set or unset instruction tree optimization (optirg) 
 *)
let process file f opti =
	try
		IrgUtil.load_with_error_support file;
		(*check ();*)
		if opti then
			Optirg.optimize (Irg.get_root ());
		let info = Toc.info () in
		f info
	with
	| Irg.PreError f
	| Irg.Error f ->
		(Irg.join_all [Irg.asis "ERROR: "; f; Irg.asis "\n"]) stderr; exit 5
	| Toc.Error msg
	| Templater.Error msg ->
		Printf.fprintf stderr "ERROR: %s\n" msg; exit 4
	| Toc.LocError (file, line, f) ->
		Printf.fprintf stderr "ERROR: %s:%d: " file line;
		f stderr;
		output_char stderr '\n';
		exit 1
	| Toc.OpError (inst, f) ->
		Printf.fprintf stderr "ERROR: "; f stderr; Printf.fprintf stderr " from instruction '%s'\n" (Iter.get_user_id inst); exit (1)
	| Sys_error msg ->
		Printf.fprintf stderr "ERROR: %s\n" msg; exit 1
	| Unix.Unix_error (err, _, path) ->
		Printf.fprintf stderr "ERROR: %s on \"%s\"\n" (Unix.error_message err) path; exit 4
		


(** Find a source from "lib/"
	@param source		Looked source.
	@param paths		List of paths to look in.
	@raise Not_found	If the source can not be found. *)
let rec find_lib source paths =
	match paths with
	| [] ->  raise Not_found
	| path::tail ->
		let path = Irg.native_path path in
		let source_path = path ^ "/" ^ source in
		if Sys.file_exists source_path then path
		else find_lib source tail


(* options *)
let nmp: string ref = ref ""
let quiet = ref false
let verbose = ref false
let optirg = ref false
let options =
	("-v", Arg.Set verbose, "verbose mode") ::
	("-q", Arg.Set quiet, "quiet mode") ::
	("-O",   Arg.Set     optirg, "try to optimize instructions tree (see doc in optirg for description)") ::
	IrgUtil.options


(** Run a standard command using IRG. Capture and display all errors.
	@param f		Function to run once the IRG is loaded.
	@param args		Added arguments.
	@param help		Help text about the command. *)
let run args help f =
	let free_arg arg =
		if !nmp = ""
		then nmp := arg
		else Printf.fprintf stderr "WARNING: only one NML file required, %s ignored\n" arg in
	Arg.parse (options @ args) free_arg help;
	if !nmp = "" then
		begin
			prerr_string "ERROR: one NML file must be given !\n";
			Arg.usage options help;
			exit 1
		end
	else
		process !nmp f !optirg


(** Build a template, possibly informing the user.
	@param template		Template name.
	@param file			File path to output to.
	@param dict			Dictionary to use. *)
let make_template template file dict =
	if not !quiet then (Printf.printf "creating \"%s\"\n" file; flush stdout);
	Templater.generate dict template file


(** Build a template, possibly informing the user.
	@param template		Template name.
	@param file			File path to output to.
	@param dict			Dictionary to use. *)
let make_template_path template file dict =
	if not !quiet then (Printf.printf "creating \"%s\"\n" file; flush stdout);
	Templater.generate_path dict template file
