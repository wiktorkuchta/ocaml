(***********************************************************************)
(*                                                                     *)
(*                           Objective Caml                            *)
(*                                                                     *)
(*            Damien Doligez, projet Para, INRIA Rocquencourt          *)
(*                                                                     *)
(*  Copyright 1997 Institut National de Recherche en Informatique et   *)
(*  en Automatique.  All rights reserved.  This file is distributed    *)
(*  under the terms of the GNU Library General Public License.         *)
(*                                                                     *)
(***********************************************************************)

(* $Id$ *)

(* Module [Weak]: arrays of weak pointers *)

type 'a t;;
        (* The type of arrays of weak pointers (weak arrays).  A weak
           pointer is an object that the garbage collector may erase at
           any time.
           A weak pointer is said to be full if it points to an object,
           empty if the object was erased by the GC.
         *)

val create : len:int -> 'a t;;
        (* [Weak.create n] returns a new weak array of length [n].
           All the pointers are initially empty.
         *)
val length : 'a t -> int;;
        (* [Weak.length ar] returns the length (number of elements) of
           [ar].
         *)
val set : 'a t -> pos:int -> 'a option -> unit;;
        (* [Weak.set ar n (Some el)] sets the [n]th cell of [ar] to be a
           (full) pointer to [el]; [Weak.set ar n None] sets the [n]th
           cell of [ar] to empty.
           Raise [Invalid_argument "Weak.set"] if [n] is not in the range
           0 to [Weak.length a - 1].
         *)
val get : 'a t -> pos:int -> 'a option;;
        (* [Weak.get ar n] returns None if the [n]th cell of [ar] is
           empty, [Some x] (where [x] is the object) if it is full.
           Raise [Invalid_argument "Weak.get"] if [n] is not in the range
           0 to [Weak.length a - 1].
         *)
val check: 'a t -> pos:int -> bool;;
        (* [Weak.check ar n] returns [true] if the [n]th cell of [ar] is
           full, [false] if it is empty.  Note that even if [Weak.check ar n]
           returns [true], a subsequent [Weak.get ar n] can return [None].
        *)
val fill: 'a t -> pos:int -> len:int -> 'a option -> unit;;
        (* [Weak.fill ar ofs len el] sets to [el] all pointers of [ar] from
           [ofs] to [ofs + len - 1].  Raise [Invalid_argument "Weak.fill"]
           if [ofs] and [len] do not designate a valid subarray of [a].
        *)
val blit : src:'a t -> src_pos:int ->
           dst:'a t -> dst_pos:int -> len:int -> unit;;
        (* [Weak.blit ar1 off1 ar2 off2 len] copies [len] weak pointers
           from [ar1] (starting at [off1]) to [ar2] (starting at [off2]).
           It works correctly even if [ar1] and [ar2] are the same.
           Raise [Invalid_argument "Weak.blit"] if [off1] and [len] do
           not designate a valid subarray of [ar1], or if [off2] and [len]
           do not designate a valid subarray of [ar2].
         *)
