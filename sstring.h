/*
 * Any strings listed in this file may be refered to with SS(name)
 * for a (ici_str_t *), and SSO(name) for an (ici_obj_t *). Maximum
 * 15 chars per string. Users need to include str.h. Wherever possible,
 * the name and the string should be the same.
 *
 * String objects made by this mechanism reside in static initialised
 * memory. They are not allocated or collected, but in other respects
 * are normal.
 *
 * This file is included with variying definitions of SSTRING()
 * from str.h (for global extern defines), and twice from sstring.c.
 *
 * NB: The declaraction of the static string structur in str.h must
 * have an s_chars[] field of large enough size to cover the longest
 * string in this file.
 */
SSTRING(struct, "struct")
SSTRING(float, "float")
SSTRING(array, "array")
SSTRING(set, "set")
SSTRING(class, "class")
SSTRING(module, "module")
SSTRING(func, "func")
SSTRING(_NULL_, "NULL")
SSTRING(in, "in")
SSTRING(onerror, "onerror")
SSTRING(else, "else")
SSTRING(auto, "auto")
SSTRING(break, "break")
SSTRING(case, "case")
SSTRING(continue, "continue")
SSTRING(default, "default")
SSTRING(do, "do")
SSTRING(extern, "extern")
SSTRING(for, "for")
SSTRING(forall, "forall")
SSTRING(if, "if")
SSTRING(return, "return")
SSTRING(static, "static")
SSTRING(switch, "switch")
SSTRING(while, "while")
SSTRING(try, "try")
SSTRING(_func_, "_func_")
SSTRING(_file_, "_file_")
SSTRING(_stdin, "stdin")
SSTRING(_stdout, "stdout")
SSTRING(_stderr, "stderr")
SSTRING(_, "_")
SSTRING(second, "second")
SSTRING(minute, "minute")
SSTRING(hour, "hour")
SSTRING(day, "day")
SSTRING(month, "month")
SSTRING(year, "year")
SSTRING(wday, "wday")
SSTRING(yday, "yday")
SSTRING(isdst, "isdst")
SSTRING(slosh0, "\\0")
SSTRING(sloshn, "\\n")
SSTRING(pattern, "pattern")
SSTRING(options, "options")
SSTRING(empty_string, "")
SSTRING(name, "name")
SSTRING(autos, "autos")
SSTRING(args, "args")
SSTRING(this, "this")
SSTRING(vargs, "vargs")
SSTRING(core, "core")
SSTRING(error, "error")
SSTRING(argc, "argc")
SSTRING(argv, "argv")
SSTRING(subject, "subject")
SSTRING(callable, "callable")
SSTRING(load, "load")
SSTRING(waitfor, "waitfor")
SSTRING(result, "result")
SSTRING(critsect, "critsect")
SSTRING(raw, "raw")
SSTRING(path, "path")
SSTRING(aggr, "aggr")
SSTRING(key, "key")
#ifndef NOSIGNALS
SSTRING(ignore, "ignore")
#endif
#ifndef NOCLASSPROTO
SSTRING(proto, "proto")
#endif
SSTRING(line, "line")


/*
 * Names of intrinsic functions (that aren't already mentioned above).
 */
SSTRING(copy, "copy")
SSTRING(exit, "exit")
SSTRING(fail, "fail")
SSTRING(int, "int")
SSTRING(eq, "eq")
SSTRING(parse, "parse")
SSTRING(string, "string")
SSTRING(typeof, "typeof")
SSTRING(push, "push")
SSTRING(pop, "pop")
SSTRING(rpush, "rpush")
SSTRING(rpop, "rpop")
SSTRING(call, "call")
SSTRING(keys, "keys")
SSTRING(vstack, "vstack")
SSTRING(tochar, "tochar")
SSTRING(toint, "toint")
SSTRING(rand, "rand")
SSTRING(interval, "interval")
SSTRING(explode, "explode")
SSTRING(implode, "implode")
SSTRING(sopen, "sopen")
SSTRING(mopen, "mopen")
SSTRING(sprintf, "sprintf")
SSTRING(currentfile, "currentfile")
SSTRING(del, "del")
SSTRING(alloc, "alloc")
SSTRING(mem, "mem")
SSTRING(nels, "nels")
SSTRING(super, "super")
SSTRING(scope, "scope")
SSTRING(isatom, "isatom")
SSTRING(gettoken, "gettoken")
SSTRING(gettokens, "gettokens")
SSTRING(num, "num")
SSTRING(assign, "assign")
SSTRING(fetch, "fetch")
SSTRING(abs, "abs")
#ifndef NOMATH
SSTRING(sin, "sin")
SSTRING(cos, "cos")
SSTRING(tan, "tan")
SSTRING(asin, "asin")
SSTRING(acos, "acos")
SSTRING(atan, "atan")
SSTRING(atan2, "atan2")
SSTRING(exp, "exp")
SSTRING(log, "log")
SSTRING(log10, "log10")
SSTRING(pow, "pow")
SSTRING(sqrt, "sqrt")
SSTRING(floor, "floor")
SSTRING(ceil, "ceil")
SSTRING(fmod, "fmod")
#endif
SSTRING(top, "top")
SSTRING(include, "include")
SSTRING(sort, "sort")
#ifdef  WHOALLOC
SSTRING(reclaim, "reclaim")
#endif
SSTRING(now, "now")
SSTRING(calendar, "calendar")
SSTRING(version, "version")
SSTRING(cputime, "cputime")
SSTRING(sleep, "sleep")
SSTRING(build, "build")
SSTRING(printf, "printf")
SSTRING(getchar, "getchar")
SSTRING(getfile, "getfile")
SSTRING(getline, "getline")
SSTRING(fopen, "fopen")
#ifndef NOPIPES
SSTRING(_popen, "popen")
#endif
SSTRING(put, "put")
SSTRING(flush, "flush")
SSTRING(close, "close")
SSTRING(seek, "seek")
#ifndef NOSYSTEM
SSTRING(system, "system")
#endif
SSTRING(eof, "eof")
SSTRING(remove, "remove")
#ifndef NODIR
SSTRING(dir, "dir")
#endif
SSTRING(getcwd, "getcwd")
SSTRING(chdir, "chdir")
SSTRING(rename, "rename")
SSTRING(eventloop, "eventloop")
SSTRING(debug, "debug")
SSTRING(new, "new")
SSTRING(isa, "isa")
SSTRING(respondsto, "respondsto")
SSTRING(profile, "profile")
SSTRING(regexp, "regexp")
SSTRING(regexpi, "regexpi")
SSTRING(sub, "sub")
SSTRING(gsub, "gsub")
SSTRING(smash, "smash")
SSTRING(signal, "signal")
SSTRING(signam, "signam")
SSTRING(pathjoin, "pathjoin")
SSTRING(core1, "core1")
SSTRING(core2, "core2")
SSTRING(core3, "core3")
SSTRING(basename, "basename")
SSTRING(pfopen, "pfopen")
SSTRING(use, "use")
SSTRING(min, "min")
SSTRING(max, "max")
SSTRING(argerror, "argerror")
SSTRING(argcount, "argcount")
SSTRING(typecheck, "typecheck")
SSTRING(pi, "pi")
SSTRING(cmp, "cmp")
SSTRING(dirname, "dirname")
SSTRING(tmpname, "tmpname")
SSTRING(strbuf, "strbuf")
SSTRING(strcat, "strcat")
SSTRING(status, "status")
SSTRING(active, "active")
SSTRING(failed, "failed")
SSTRING(finished, "finished")
SSTRING(parser, "parser")
SSTRING(parsetoken, "parsetoken")
SSTRING(tokenobj, "tokenobj")
SSTRING(rejecttoken, "rejecttoken")
SSTRING(parsevalue, "parsevalue")
SSTRING(rejectchar, "rejectchar")
SSTRING(which, "which")
#if 0
    SSTRING(parse_expr, "parse_expr")
    SSTRING(parse_stmt, "parse_stmt")
#endif
