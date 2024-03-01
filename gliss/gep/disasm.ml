(*
 * GLISS2 -- disassembly module
 * Copyright (c) 2008, IRIT - UPS <casse@irit.fr>
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

open Irg
open Printf

(** Generate code to perform disassembly.
	@param out	Output channel.
	@param inst	Current instruction.
	@param expr	Syntax expression.
	@raise Error	If there is an unsupported syntax expression. *)
let rec gen info inst expr =

	let str text = Irg.CONST (Irg.STRING, Irg.STRING_CONST(text)) in

	let dec_format f a =
		(String.sub f 0 ((String.length f) - 1)) ^
		(match Toc.convert_type (Sem.get_type_expr a) with
		| Toc.INT8
		| Toc.UINT8
		| Toc.INT16
		| Toc.UINT16
		| Toc.INT32		-> "d"
		| Toc.UINT32	-> "u"
		| Toc.INT64		-> "Ld"
		| Toc.UINT64	-> "Lu"
		| Toc.FLOAT
		| Toc.DOUBLE
		| Toc.LONG_DOUBLE
		| Toc.CHAR_PTR	-> failwith "gen_disasm.int_format") in

	let unsigned_format i i32 i64 f a =
		(String.sub f 0 ((String.length f) - 1)) ^
		(match Toc.convert_type (Sem.get_type_expr a) with
		| Toc.INT8
		| Toc.UINT8
		| Toc.INT16
		| Toc.UINT16	-> i
		| Toc.INT32
		| Toc.UINT32 	-> i32
		| Toc.INT64
		| Toc.UINT64 	-> i64
		| Toc.FLOAT
		| Toc.DOUBLE
		| Toc.LONG_DOUBLE
		| Toc.CHAR_PTR 	-> failwith "gen_disasm.int_format") in

	let fix f a =
		(match String.get f ((String.length f) - 1) with
		| 'd' | 'i' | 'u'	-> dec_format
		| 'x' 				-> unsigned_format "x" "x" "Lx"
		| 'X' 				-> unsigned_format "X" "X" "LX"
		| 'o' 				-> unsigned_format "o" "lo" "Lo"
		| 'b' 				-> unsigned_format "b" "lb" "Lb"
		| _ 				-> (fun f _ -> f)) f a in

	let format fmt args =
		if fmt = "" then Irg.NOP else
        Irg.CANON_STAT ("__buffer += sprintf", (Irg.REF (Irg.NO_TYPE, "__buffer"))::(str fmt)::args) in

	let buffer = Irg.REF (Irg.NO_TYPE, "__buffer") in
	let sprintf = "__buffer += sprintf" in
	let sformat = Irg.CONST(Irg.STRING, Irg.STRING_CONST "%s") in

	let bad_expr e =
		Irg.error (fun out -> fprintf out "bad syntax expression in instruction %s: %a"
			(Iter.get_user_id inst) Irg.output_expr e) in

	let rec scan fmt args cfmt cargs =
		match fmt with
		| []
			-> format cfmt (List.rev cargs)
		| (Str.Text t)::fmt
			-> scan fmt args (cfmt ^ t) cargs
		| (Str.Delim d)::fmt when (Str.last_chars d 1) = "s"
	 		-> Irg.SEQ (format cfmt (List.rev cargs), Irg.SEQ(process (List.hd args), scan fmt (List.tl args) "" []))
		| (Str.Delim d)::fmt when (Str.last_chars d 1) = "@"
			-> scan fmt (List.tl args) (cfmt ^ "%s") (((Irg.CANON_EXPR(Irg.STRING, info.Toc.proc ^ "_solve_label", [List.hd args])))::cargs)
		| (Str.Delim d)::fmt
			-> scan fmt (List.tl args) (cfmt ^ (fix d (List.hd args))) ((List.hd args)::cargs)

	and process expr =
		check expr;
		match expr with
		| Irg.FORMAT (fmt, args) ->
			scan (Irg.split_format_string fmt) args "" []
		| Irg.CONST (_, Irg.STRING_CONST(s)) ->
    		if s <> "" then Irg.CANON_STAT (sprintf, [buffer; str s]) else Irg.NOP
		| Irg.IF_EXPR (_, c, t, e) ->
			Irg.IF_STAT(c, process t, process e)
		| Irg.SWITCH_EXPR(_, c, cases, def) ->
			Irg.SWITCH_STAT(c, List.map (fun (c, e) -> (c, process e)) cases,if def <> Irg.NONE then process def else Irg.NOP)
		| Irg.CANON_EXPR _ ->
			Irg.CANON_STAT(sprintf, [buffer; sformat; expr])
		| Irg.ELINE (f, l, e) ->
			Irg.handle_error f l (fun _ -> gen info inst e)
		| Irg.REF (_, id) ->
			(match Irg.get_symbol id with
			| ATTR (ATTR_EXPR (_, e)) -> process e
			| _ -> bad_expr expr)
		| Irg.FIELDOF (t, cid, fid) ->
			(match Irg.get_symbol cid with
			| ATTR (ATTR_EXPR (_, e)) ->
				(match escape_eline e with
				| REF (_, id) -> process (FIELDOF(t, id, fid))
				| _ -> bad_expr expr)
			| _ ->
				bad_expr expr)
		| Irg.NONE
		| Irg.ITEMOF _
		| Irg.BITFIELD _
		| Irg.UNOP _
		| Irg.BINOP _
		| Irg.CONST _
		| Irg.COERCE _
		| Irg.CAST _ ->
			bad_expr expr

	and check_symbol id =
		match Irg.get_symbol id with
		| REG _ | MEM _ ->
			Irg.error_spec inst
				(fun out -> fprintf out "\"%s\" forbidden in syntax attribute: %a" id output_expr expr)
		| LET _ | Irg.PARAM _  | _ -> ()

	and check expr =
		match expr with
		| Irg.NONE ->
			()
		| Irg.COERCE (_, expr) ->
			check expr
		| Irg.FORMAT (_, args)
		| Irg.CANON_EXPR (_, _, args) ->
			List.iter check args
		| Irg.REF (_, id) ->
			check_symbol id
		| Irg.FIELDOF (_, id, _) ->
			check_symbol id
		| Irg.ITEMOF (_, id, expr) ->
			check_symbol id; check expr
		| Irg.BITFIELD (_, b, l, u) ->
			check b; check l; check u
		| Irg.UNOP (_, _, arg) ->
			check arg
		| Irg.BINOP (_, _, arg1, arg2) ->
			check arg1; check arg2
		| Irg.IF_EXPR (_, c, t, e) ->
			check c; check t; check e
		| Irg.SWITCH_EXPR (_, c, cs, d) ->
			check c; check d; List.iter (fun (_, e) -> check e) cs
		| Irg.CONST _ -> ()
		| Irg.ELINE (f, l, e) ->
			Irg.handle_error f l (fun _ -> Toc.locate_error f l check e)
		| Irg.CAST (_, e) ->
			check e in

	(* !!DEBUG!! *)
	(*print_string "gen_disasm:";
	Irg.print_expr expr;
	print_char '\n';*)
	process expr
