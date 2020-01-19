# For complete documentation of this file, please see Geany's main documentation

[styling]
# foreground;background;bold;italic
default=default
comment=comment
commentline=comment
commentdoc=commentdoc
number=number
word=keyword
word2=keyword2
string=string
character=string
uuid=extra
preprocessor=preprocessor
operator=operator
identifier=default
stringeol=stringeol
# @"verbatim"
verbatim=extra
# (/regex/)
regex=extra
commentlinedoc=commentdoc,bold
commentdockeyword=commentdoc,bold,italic
commentdockeyworderror=commentdoc
globalclass=type

[keywords]
# all items must be in one line
primary=break case catch conf const continue default delete do each else false finally for function get if in Infinity instanceof let NaN names new null return set switch this throw true try typeof undefined var void while with yield CData CStruct CEnum CType Sys Zvfs console Info FileOp log puts source signal setTimeout setInterval clearInterval exit load assert quote format self this

secondary=Array Boolean Date Event Function Math Number Object Socket String RegExp EvalError Error Event RangeError ReferenceError Sqlite Signal SyntaxError TypeError URIError WebSocket prototype decodeURI decodeURIComponent encodeURI encodeURIComponent eval isFinite isNaN parseFloat parseInt Sqlite Websocket JSON Interp File

[settings]
# default extension used when saving files
extension=js

# the following characters are these which a "word" can contains, see documentation
#wordchars=_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789

# single comments, like # in this file
comment_single=//
# multiline comments
comment_open=/*
comment_close=*/

# set to false if a comment character/string should start at column 0 of a line, true uses any
# indentation of the line, e.g. setting to true causes the following on pressing CTRL+d
    #command_example();
# setting to false would generate this
#   command_example();
# This setting works only for single line comments
comment_use_indent=true

# context action command (please see Geany's main documentation for details)
context_action_cmd=

[indentation]
#width=4
# 0 is spaces, 1 is tabs, 2 is tab & spaces
#type=1

[build_settings]
# %f will be replaced by the complete filename
# %e will be replaced by the filename without extension
# (use only one of it at one time)
compiler=
run=


[build-menu]
FT_00_LB=jsish
FT_00_CM=jsish "%f"
FT_00_WD=
FT_01_LB=exec
FT_01_CM=exec "./%f"
FT_01_WD=
