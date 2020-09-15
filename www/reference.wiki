# Reference
The generated reference for Jsi builtin commands.



## Array



Provide access to array objects.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Array|(...):array |jsi_Array constructor.|
|concat|(...):array |Return array with args appended.|
|every|(callback:function) |Returns true if every value in array satisfies the test.|
|fill|(value:any, start:number=0, end:number=-1):array |Fill an array with values.|
|filter|(callback:function, thisArg:object=void):array |Return a filtered array.|
|find|(callback:function) |Returns the value of the first element in the array that satisfies the test.|
|findIndex|(callback:function) |Returns the index of the first element in the array that satisfies the test.|
|flat|(depth:number=1):array |Flatten an arra.|
|forEach|(callback:function, thisArg:object=void):void |Invoke function with each item in object.|
|includes|(val:any) |Returns true if array contains value.|
|indexOf|(str:any, startIdx:number=0):number |Return index of first occurrance in array.|
|isArray|():boolean |True if val array.|
|join|(sep:string=''):string |Return elements joined by char.|
|lastIndexOf|(val:any, start:number=0):number |Return index of last occurence in array.|
|map|(callback:function, thisArg:object=void):array |Creates a new array with the results of calling a provided function on every element in this array.|
|pop|() |Remove and return last element of array.|
|push|(val:any, ...):number |Push one or more elements onto array and return size.|
|reduce|(callback:function, initial:any) |Return a reduced array.|
|reduceRight|(callback:function, initial:any) |Return a reduced array.|
|reverse|():array |Reverse order of all elements in an array.|
|shift|() |Remove first element and shift downwards.|
|sizeOf|():number |Return size of array.|
|slice|(start:number, end:number=void):array |Return sub-array.|
|some|(callback:function, thisArg:object=void):boolean |Return true if function returns true some element.|
|sort|([options](#options-arraysort):function&#124;object=void):array |Sort an array.|
|splice|(start:number, howmany:number=void, ...):array |Change the content of an array, adding new elements while removing old elements.|
|unshift|(...):number |Add new elements to start of array and return size.|
### Options "Array.sort"
|Option|Type|Description|Flags|
|---|---|---|---|
|mode|*STRKEY*|Mode to sort by. (one of: **default**, **desc**, **dict**, **nocase**)||
|compare|*FUNC*|Function to do comparison. @`function(val1,val2)`||
|unique|*BOOL*|Eliminate duplicate items.||


## Boolean



A Boolean object.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Boolean|(bool:boolean=false):boolean |Boolean constructor.|


## CData




### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|CData|([options](#options-newcdata):string&#124;object=void, inits:object=undefined):userobj |Create a new struct or map/array of structs.The 2nd arg is used for function option parsing and will report errors at the callers file:line|
|conf|([options](#options-cdataconf):object&#124;string=void) |Configure options for c-data.|
|get|(key:string&#124;number&#124;object=null, field:string=void) |Get struct/map/array value.|
|incr|(key:string&#124;number&#124;object&#124;null, field:object&#124;string, value:number=1):number |Increment a numeric field: returns the new value.|
|info|():object |Return info for data.|
|names|():array |Return keys for map.|
|set|(key:string&#124;number&#124;object&#124;null, field:object&#124;string, value:any=void) |Set a struct/map/array value.|
|unset|(key:string&#124;number&#124;object) |Remove entry from map/array.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|arrSize|*UINT*|If an array, its size in elements.|initOnly|
|flags|*UINT*|Flags.|initOnly|
|help|*STRKEY*|Description of data.|initOnly|
|keyName|*STRKEY*|Key struct, for key struct maps.|initOnly|
|keyType|*STRKEY*|Key id. (one of: **string**, **strkey**, **number**)|initOnly|
|mapType|*STRKEY*|If a map, its type. (one of: **none**, **hash**, **tree**, **list**)|initOnly|
|maxSize|*UINT*|Limit the array size or number of keys in a map.||
|name|*STRKEY*|Name (eg. of var assigned to on create).|initOnly|
|noAuto|*BOOL*|Disable auto-create of map keys in set/incr.||
|structName|*STRKEY*|Struct used for storing data.|initOnly&#124;required|
|user|*INT64*|User data.||
|varParam|*STRKEY*|Param for maps/array vars.|initOnly|


## CEnum



Enum commands. Note: Enum() is a shortcut for Enum.add().


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|add|([options](#options-cenumadd):object&#124;string, fields:array&#124;string) |Create a new enum: value of items same as in fieldconf.|
|conf|([options](#options-cenumconf):object&#124;string=void) |Configure options for enum.|
|fieldconf|([options](#options-cenumfieldconf):object&#124;string=void) |Configure options for fields.|
|find|(enum:string, intValue:number):string |Find item with given value in enum.|
|get|(enum:string):object |Return enum definition.|
|names|(enum:string=void):array |Return name list of all enums, or items within one enum.|
|remove|(enum:string) |Remove an enum.|
|value|(enum:string, item:string):number |Return value for given enum item.|
### Options "CEnum.add"
|Option|Type|Description|Flags|
|---|---|---|---|
|flags|*INT64*|Flags for enum.|initOnly|
|help|*STRKEY*|Description of enum.|initOnly|
|name|*STRKEY*|Name of enum.|initOnly|
|idx|*UINT*|Number of items in enum.|readOnly|
### Options "CEnum.conf"
|Option|Type|Description|Flags|
|---|---|---|---|
|flags|*INT64*|Flags for enum.|initOnly|
|help|*STRKEY*|Description of enum.|initOnly|
|name|*STRKEY*|Name of enum.|initOnly|
|idx|*UINT*|Number of items in enum.|readOnly|
### Options "CEnum.fieldconf"
|Option|Type|Description|Flags|
|---|---|---|---|
|flags|*INT64*|Flags for item.|initOnly|
|help|*STRKEY*|Desciption of item.|initOnly|
|name|*STRKEY*|Name of item.|initOnly|
|value|*INT64*|Value for item.|initOnly|
|idx|*UINT*|Index of item in enum.|readOnly|


## CStruct



Struct commands. Note: Struct() is a shortcut for Struct.add().


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|add|([options](#options-cstructadd):object&#124;string, fields:array&#124;string) |Create a struct: field values same as in fieldconf.|
|conf|([options](#options-cstructconf):object&#124;string=void) |Configure options for struct.|
|fieldconf|([options](#options-cstructfieldconf):object&#124;string=void) |Configure options for fields.|
|get|(struct, options:object=void):object |Return the struct definition.|
|names|(struct:string=void):array |Return name list of all structs, or fields for one struct.|
|remove|(name:string) |Remove a struct.|
|schema|():string |Return database schema for struct.|
### Options "CStruct.add"
|Option|Type|Description|Flags|
|---|---|---|---|
|crc|*UINT32*|Crc for struct.|initOnly|
|flags|*INT64*|Flags for struct.|initOnly|
|help|*STRKEY*|Struct description.|initOnly|
|idx|*UINT32*|Number of fields in struct.|readOnly|
|name|*STRKEY*|Name of struct.|initOnly&#124;required|
|size|*UINT*|Size of struct in bytes.|readOnly|
|ssig|*UINT32*|Signature for struct.|initOnly|
|value|*INT64*|Reference count.|readOnly|
### Options "CStruct.conf"
|Option|Type|Description|Flags|
|---|---|---|---|
|crc|*UINT32*|Crc for struct.|initOnly|
|flags|*INT64*|Flags for struct.|initOnly|
|help|*STRKEY*|Struct description.|initOnly|
|idx|*UINT32*|Number of fields in struct.|readOnly|
|name|*STRKEY*|Name of struct.|initOnly&#124;required|
|size|*UINT*|Size of struct in bytes.|readOnly|
|ssig|*UINT32*|Signature for struct.|initOnly|
|value|*INT64*|Reference count.|readOnly|
### Options "CStruct.fieldconf"
|Option|Type|Description|Flags|
|---|---|---|---|
|arrSize|*UINT*|Size of field if an array.|initOnly|
|bits|*UINT32*|Size of bitfield.|initOnly|
|boffset|*UINT32*|Bit offset of field within struct.|readOnly|
|flags|*INT64*|Flags for field.|initOnly|
|idx|*UINT32*|Index of field in struct.|readOnly|
|help|*STRKEY*|Field description.|initOnly|
|info|*STRKEY*|Info for field.|initOnly|
|name|*STRKEY*|Name of field.|initOnly&#124;required|
|offset|*UINT*|Offset of field within struct.|readOnly|
|size|*UINT*|Size of field in struct.|readOnly|
|type|*CUSTOM*|Type of field.|initOnly&#124;required|
|init|*CUSTOM*|Initial value for field.|initOnly|


## CType



Type commands. Note: Type() is a shortcut for Type.conf().


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|conf|([options](#options-ctypeconf):object&#124;string=void) |Configure options for type.|
|names|(ctype=false):array |Return type names.|
### Options "CType.conf"
|Option|Type|Description|Flags|
|---|---|---|---|
|idName|*STRKEY*|The id name: usually upcased cName.|initOnly|
|cName|*STRKEY*|C type name.|initOnly|
|help|*STRKEY*|Description of id.|initOnly|
|fmt|*STRKEY*|Printf format for id.|initOnly|
|xfmt|*STRKEY*|Hex printf format for id.|initOnly|
|flags|*INT64*|Flags for id.|initOnly|
|size|*INT*|Size for id.|initOnly|
|user|*INT64*|User data.||


## Channel



Commands for accessing Channel objects for file IO.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Channel|(file:string, mode:string='r'):userobj |A file input/output object. The mode string is r or w and an optional +.|
|close|():boolean |Close the file.|
|eof|():boolean |Return true if read to end-of-file.|
|filename|():string |Get file name.|
|flush|():number |Flush file output.|
|gets|():string&#124;void |Get one line of input.|
|lstat|():object |Return status for file.|
|mode|():string |Get file mode used with open.|
|open|(file:string, mode:string='r'):boolean |Open the file (after close).|
|puts|(str):boolean |Write one line of output.|
|read|(size:number=-1):string&#124;void |Read some or all of file.|
|seek|(pos:number, whence:string):number |Seek to position. Return 0 if ok.|
|stat|():object |Return status for file.|
|tell|():number |Return current position.|
|truncate|(pos:number):number |Truncate file.|
|write|(data):number |Write data to file.|


## Debugger



Debugger breakpoint management.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|add|(val:string&#124;number, temp:boolean=false):number |Add a breakpoint for line, file:line or func.|
|enable|(id:number, on:boolean):void |Enable/disable breakpoint.|
|info|(id:number=void):array&#124;object |Return info about one breakpoint, or list of bp numbers.|
|remove|(id:number):void |Remove breakpoint.|


## Event



Event management.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|clearInterval|(id:number):void |Delete an event (created with setInterval/setTimeout).|
|info|(id:number):object |Return info for the given event id.|
|names|():array |Return list event ids (created with setTimeout/setInterval).|
|setInterval|(callback:function, millisecs:number):number |Setup recurring function to run every given millisecs.|
|setTimeout|(callback:function, millisecs:number):number |Setup function to run after given millisecs.|
|update|([options](#options-eventupdate):number&#124;object=void):number |Service all events, eg. setInterval/setTimeout. Returns the number of events processed. Events are processed until minTime (in milliseconds) is exceeded, or forever if -1. The default minTime is 0, meaning return as soon as no events can be processed. A positive mintime will result in sleeps between event checks.|
### Options "Event.update"
|Option|Type|Description|Flags|
|---|---|---|---|
|maxEvents|*INT*|Maximum number of events to process (or -1 for all).||
|maxPasses|*INT*|Maximum passes through event queue.||
|minTime|*INT*|Minimum milliseconds before returning, or -1 to loop forever (default is 0).||
|sleep|*INT*|Time to sleep time (in milliseconds) between event checks. Default is 1.||


## File



Commands for accessing the filesystem.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|atime|(file:string):number |Return file Jsi_Access time.|
|chdir|(file:string) |Change current directory.|
|chmod|(file:string, mode:number) |Set file permissions.|
|copy|(src:string, dest:string, force:boolean=false) |Copy a file to destination. Directories are not handled. The third argument if given is a boolean force value which if true allows overwrite of an existing file. |
|dirname|(file:string):string |Return directory path.|
|executable|(file:string):boolean |Return true if file is executable.|
|exists|(file:string):boolean |Return true if file exists.|
|extension|(file:string):string |Return file extension.|
|glob|([options](#options-fileglob):function&#124;object&#124;null=void):array |Return list of files in dir with optional pattern match. With no arguments (or null) returns all files/directories in current directory. The first argument can be a pattern (either a glob or regexp) of the files to return. When the second argument is a function, it is called with each path, and filter on false. Otherwise second argument must be a set of options.|
|isdir|(file:string):boolean |Return true if file is a directory.|
|isfile|(file:string):boolean |Return true if file is a normal file.|
|isrelative|(file:string):boolean |Return true if file path is relative.|
|join|(path1:string, path2:string):string |Join two file realpaths, or just second if an absolute path.|
|link|(src:string, dest:string, ishard:boolean=false) |Link a file. The second argument is the destination file to be created. If a third bool argument is true, a hard link is created.|
|lstat|(file:string):object |Return status info for file.|
|mkdir|(file:string,force:boolean=false) |Create a directory: force creates subdirs.|
|mknod|(file:string, mode:number, dev:number) |Create unix device file using mknod.|
|mtime|(file:string):number |Return file modified time.|
|owned|(file:string):boolean |Return true if file is owned by user.|
|perms|(file:string):string |Return perms string.|
|pwd|():string |Return current directory.|
|read|(file:string, mode:string='rb'):string |Read a file.|
|readable|(file:string):boolean |Return true if file is readable.|
|readlink|(file:string):string |Read file link destination.|
|realpath|(file:string):string |Return absolute file name minus .., ./ etc.|
|remove|(file:string, force:boolean=false) |Delete a file or direcotry.|
|rename|(src:string, dest:string, force:boolean=false) |Rename a file, with possible overwrite.|
|rootname|(file:string):string |Return file name minus extension.|
|size|(file:string):number |Return size for file.|
|stat|(file:string):object |Return status info for file.|
|tail|(file:string):string |Return file name minus dirname.|
|tempfile|(file:string) |Create a temp file.|
|truncate|(file:string, size:number) |Truncate file.|
|writable|(file:string):boolean |Return true if file is writable.|
|write|(file:string, str:string, mode:string='wb+'):number |Write a file.|
### Options "File.glob"
|Option|Type|Description|Flags|
|---|---|---|---|
|dir|*STRING*|The start directory: this path will not be prepended to results.||
|maxDepth|*INT*|Maximum directory depth to recurse into.||
|maxDiscard|*INT*|Maximum number of items to discard before giving up.||
|dirFilter|*FUNC*|Filter function for directories, returning false to discard. @`function(dir:string)`||
|filter|*FUNC*|Filter function to call with each file, returning false to discard. @`function(file:string)`||
|limit|*INT*|The maximum number of results to return/count: -1 is unlimited (Interp.maxArrayList).||
|noTypes|*STRKEY*|Filter files to exclude these "types".||
|prefix|*STRKEY*|String prefix to prepend to each file in result list.||
|recurse|*BOOL*|Recurse into sub-directories.||
|retCount|*BOOL*|Return only the count of matches.||
|retInfo|*BOOL*|Return file info: size, uid, gid, mode, name, and path.||
|tails|*BOOL*|Returned only tail of path.||
|types|*STRKEY*|Filter files to include type: one or more of chars 'fdlpsbc' for file, directory, link, etc.||


## Function



Commands for accessing functions.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Function|():function |Function constructor (unimplemented).|
|apply|(thisArg:null&#124;object&#124;function, args:array=void) |Call function passing args array.|
|bind|(thisArg:object&#124;function=null,...) |Return function that calls bound function prepended with thisArg+arguments.|
|call|(thisArg:null&#124;object&#124;function, arg1, ...) |Call function with args.|


## Info



Commands for inspecting internal state information in JSI.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|argv0|():string&#124;void |Return initial start script file name.|
|cmds|([options](#options-infocmds):object=void):array&#124;object |Return details or list of matching commands.|
|completions|(str:string, start:number=0, end:number=void):array |Return command completions on portion of string from start to end.|
|data|(val:string&#124;regexp&#124;object=void):array&#124;object |Return list of matching data (non-functions). Like info.vars(), but does not return function values.|
|error|():object |Return file and line number of error (used inside catch).|
|event|(id:number=void):array&#124;object |List events or info for 1 event (setTimeout/setInterval). With no args, returns list of all outstanding events.  With one arg, returns infofor the given event id.|
|execZip|():string&#124;void |If executing a .zip file, return file name.|
|executable|():string |Return name of executable.|
|files|():array |Return list of all sourced files.|
|funcs|(arg:string&#124;regexp&#124;function&#124;object=void):array&#124;object |Return details or list of matching functions.|
|interp|(interp:userobj=void):object |Return info on given or current interp.|
|isMain|():boolean |Return true if current script was the main script invoked from command-line.|
|keywords|(isSql=false, name:string=void):boolean&#124;array |Return/lookup reserved keyword.|
|level|(level:number=void):number&#124;array&#124;object |Return current level or details of a call-stack frame. With no arg, returns the number of the current stack frame level. Otherwise returns details on the specified level. The topmost level is 1, and 0 is the current level, and a negative level translates as relative to the current level.|
|locals|(filter:boolean=void):object |Return locals; if filter=true/false omit vars/functions.|
|lookup|(name:string) |Given string name, lookup and return value, eg: function.|
|methods|(val:string&#124;regexp):array&#124;object |Return functions and commands.|
|named|(name:string=void):array&#124;userobj |Returns command names for builtin Objects, eg: 'File', 'Interp', sub-Object names, or the named object.|
|options|(ctype:boolean=false):array |Return Option type name, or with true the C type.|
|package|(pkgName:string):object&#124;null |Return info about provided package if exists, else null.|
|platform|():object |N/A. Returns general platform information for JSI.|
|script|(func:function&#124;regexp=void):string&#124;array&#124;void |Get current script file name, or file containing function.|
|scriptDir|():string&#124;void |Get directory of current script.|
|vars|(val:string&#124;regexp&#124;object=void):array&#124;object |Return details or list of matching variables. Returns all values, data or function.|
|version|(full:boolean=false):number&#124;object |JSI version: returns object when full=true.|
### Options "Info.cmds"
|Option|Type|Description|Flags|
|---|---|---|---|
|full|*BOOL*|Return full path.||
|constructor|*BOOL*|Do not exclude constructor.||


## Interp



Commands for accessing interps.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Interp|([options](#options-newinterp):object=void):userobj |Create a new interp.|
|alias|(name:string=void, func:function&#124;null=void, args:array&#124;null=void, async=false) |Set/get global alias bindings for command in an interp. With 0 args, returns list of all aliases in interp. With 1 arg returns func for given alias name. With 2 args where arg2 == null, returns args for given alias name . With 3 args, create/update an alias for func and args.  Delete an alias by creating it with null for both func and args.|
|call|(funcName:string, args:array, wait:boolean=false) |Call named function in subinterp. Invoke function in sub-interp with arguments. Since interps are not allowed to share objects, data is automatically cleansed by encoding/decoding to/from JSON if required. Unless an 'async' parameter is true call is acyncronous. Otherwise waits until the sub-interp is idle, to make call and return result.|
|conf|([options](#options-interpconf):string&#124;object=void) |Configure option(s).|
|eval|(js:string, async:boolean=false) |Interpret script within sub-interp. When the 'async' option is used on a threaded interp, the script is queued as an Event.|
|info|():object |Returns internal statistics about interp.|
|source|(file:string, async:boolean=false) |Interpret file within sub-interp. When the 'async' option is used on a threaded interp, the script is queued as an Event.|
|uplevel|(js:string, level:number=0) |Interpret code at the given stack level. The level argument is as returned by Info.level().  Not supported with threads.|
|value|(varName:string, level:number=0) |Lookup value of variable at stack level.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|args|*ARRAY*|The console.arguments for interp.|initOnly|
|assertMode|*STRKEY*|Action upon assert failure. (one of: **throw**, **log**, **puts**)||
|autoFiles|*ARRAY*|File(s) to source for loading Jsi_Auto to handle unknown commands.||
|busyCallback|*CUSTOM*|Command in parent interp (or noOp) to periodically call.||
|busyInterval|*INT*|Call busyCallback command after this many op-code evals (100000).||
|confFile|*STRKEY*|Config file of options in JSON non-strict format.|initOnly|
|coverage|*BOOL*|On exit generate detailed code coverage for function calls (with profile).||
|debugOpts|*[Options](#options-debugOpts)*|Options for debugging.||
|interactive|*BOOL*|Force interactive mode. ie. ignore no_interactive flag.|initOnly|
|hasOpenSSL|*BOOL*|WebSocket compiled with SSL is available.|initOnly|
|historyFile|*STRKEY*|For readline, file to use for history (~/.jsish_history).||
|isSafe|*BOOL*|Is this a safe interp (ie. with limited or no file access).|initOnly|
|jsppChars|*STRKEY*|Line preprocessor when sourcing files. Line starts with first char, and either ends with it, or matches string.||
|jsppCallback|*FUNC*|Command to preprocess lines that match jsppChars. Call func(interpName:string, opCnt:number).||
|lockTimeout|*INT*|Thread time-out for mutex lock acquires (milliseconds).||
|lockDown|*STRKEY*|Directory to Safe-lockdown interpreter to.||
|logOpts|*[Options](#options-logOpts)*|Options for log output to add file/line/time.||
|log|*ARRAY*|Logging flags. (zero or more of: **bug**, **assert**, **debug**, **trace**, **test**, **info**, **warn**, **error**, **parse**)|noCase|
|maxDepth|*INT*|Depth limit of recursive function calls (1000).||
|maxDumpStack|*UINT*|Maximum stack dump length (100).||
|maxDumpArgs|*UINT*|Maximum arg length in stack dump (80).||
|maxArrayList|*UINT*|Maximum array convertable to list (100000).||
|maxIncDepth|*INT*|Maximum allowed source/require nesting depth (50).||
|maxInterpDepth|*INT*|Maximum nested subinterp create depth (10).||
|maxUserObjs|*INT*|Maximum number of 'new' object calls, eg. File, RegExp, etc.||
|maxOpCnt|*INT*|Execution limit for op-code evaluation.|initOnly|
|memDebug|*INT*|Memory debugging level: 1=summary, 2=detail.||
|memLeakCnt|*INT*|Leak memory count due to object added to self.|initOnly|
|name|*STRKEY*|Optional text name for this interp.||
|noAutoLoad|*BOOL*|Disable autoload.||
|noConfig|*BOOL*|Disable use of Interp.conf to change options after create.|initOnly|
|noEval|*BOOL*|Disable eval: just parses file to check syntax.|initOnly|
|noInput|*BOOL*|Disable use of console.input().||
|noLoad|*BOOL*|Disable load of shared libs.||
|noNetwork|*BOOL*|Disable new Socket/WebSocket, or load of builtin MySql.||
|noStderr|*BOOL*|Make puts, log, assert, etc use stdout.||
|noSubInterps|*BOOL*|Disallow sub-interp creation.||
|onComplete|*FUNC*|Function to return commands completions for interactive mode.  Default uses Info.completions . @`function(prefix:string, start:number, end:number)`||
|onEval|*FUNC*|Function to get control for interactive evals. @`function(cmd:string)`||
|onExit|*FUNC*|Command to call in parent on exit, returns true to continue. @`function()`|initOnly|
|pkgDirs|*ARRAY*|list of library directories for require() to search.||
|profile|*BOOL*|On exit generate profile of function calls.||
|retValue|*VALUE*|Return value from last eval.|readOnly|
|safeMode|*STRKEY*|In safe mode source() support for pwd and script-dir . (one of: **none**, **read**, **write**, **writeRead**, **lockdown**)|initOnly|
|safeReadDirs|*ARRAY*|In safe mode, files/dirs to allow reads to.|initOnly|
|safeWriteDirs|*ARRAY*|In safe mode, files/dirs to allow writes to.|initOnly|
|safeExecPattern|*STRKEY*|In safe mode, regexp pattern allow exec of commands.|initOnly|
|scriptStr|*STRKEY*|Interp init script string.|initOnly|
|scriptFile|*STRING*|Interp init script file.||
|stdinStr|*STRING*|String to use as stdin for console.input().||
|stdoutStr|*STRING*|String to collect stdout for puts().||
|subOpts|*[Options](#options-subOpts)*|Infrequently used sub-options.||
|subthread|*BOOL*|Create a threaded Interp.|initOnly|
|traceCall|*ARRAY*|Trace commands. (zero or more of: **funcs**, **cmds**, **new**, **return**, **args**, **notrunc**, **noparent**, **full**, **before**)||
|traceOp|*INT*|Set debugging level for OPCODE execution.||
|tracePuts|*BOOL*|Trace puts by making it use logOpts.||
|typeCheck|*ARRAY*|Type-check control options. (zero or more of: **none**, **parse**, **run**, **all**, **error**, **strict**, **noundef**, **nowith**, **funcsig**)||
|typeWarnMax|*INT*|Type checking is silently disabled after this many warnings (50).||
|udata|*OBJ*|User data.||
|unitTest|*UINT*|Unit test control bits: 1=subst, 2=Puts with file:line prefix.||
### Options "debugOpts"
|Option|Type|Description|Flags|
|---|---|---|---|
|debugCallback|*CUSTOM*|Command in parent interp for handling debugging.||
|doContinue|*BOOL*|Continue execution until breakpoint.||
|forceBreak|*BOOL*|Force debugger to break.||
|includeOnce|*BOOL*|Source the file only if not already sourced.||
|includeTrace|*BOOL*|Trace includes.||
|minLevel|*INT*|Disable eval callback for level higher than this.||
|msgCallback|*CUSTOM*|Comand in parent interp to handle Jsi_LogError/Jsi_LogWarn,...||
|pkgTrace|*BOOL*|Trace package loads.||
|putsCallback|*CUSTOM*|Comand in parent interp to handle puts output.||
|traceCallback|*CUSTOM*|Comand in parent interp to handle traceCall.||
|testFmtCallback|*CUSTOM*|Comand in parent interp to format unittest string.||
### Options "logOpts"
|Option|Type|Description|Flags|
|---|---|---|---|
|time|*BOOL*|Prefix with time.||
|date|*BOOL*|Prefix with date.||
|file|*BOOL*|Ouptut contains file:line.||
|func|*BOOL*|Output function.||
|full|*BOOL*|Show full file path.||
|ftail|*BOOL*|Show tail of file only, even in LogWarn, etc.||
|before|*BOOL*|Output file:line before message string.||
|isUTC|*BOOL*|Time is to be UTC.||
|timeFmt|*STRKEY*|A format string to use with strftime.||
|chan|*USEROBJ*|Channel to send output to.||
### Options "subOpts"
|Option|Type|Description|Flags|
|---|---|---|---|
|blacklist|*STRKEY*|Comma separated modules to disable loading for.|initOnly|
|compat|*BOOL*|Ignore unknown options via JSI_OPTS_IGNORE_EXTRA in option parser.||
|dblPrec|*INT*|Format precision of double where 0=max, -1=max-1, ... (max-1).||
|istty|*BOOL*|Indicates interp is in interactive mode.|readOnly|
|nofreeze|*BOOL*|moduleOpts freeze disabled by default.||
|logColNums|*BOOL*|Display column numbers in error messages.||
|logAllowDups|*BOOL*|Log should not filter out duplicate messages.||
|mutexUnlock|*BOOL*|Unlock own mutex when evaling in other interps (true).|initOnly|
|noproto|*BOOL*|Disable support of the OOP symbols:  __proto__, prototype, constructor, etc.||
|noFuncString|*BOOL*|Disable viewing code body for functions.|initOnly|
|noRegex|*BOOL*|Disable viewing code for functions.|initOnly|
|noReadline|*BOOL*|In interactive mode disable use of readline.||
|outUndef|*BOOL*|In interactive mode output result values that are undefined.||
|prompt|*STRKEY*|Prompt for interactive mode ('$ ').||
|prompt2|*STRKEY*|Prompt for interactive mode line continue ('> ').||


## JSON



Commands for handling JSON data.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|check|(str:string, strict:boolean=true):boolean |Return true if str is JSON.|
|parse|(str:string, strict:boolean=true) |Parse JSON and return js.|
|stringify|(value:any,  strict:boolean=true):string |Return JSON from a js object.|


## Math



Commands performing math operations on numbers.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|abs|(num:number):number |Returns the absolute value of x.|
|acos|(num:number):number |Returns the arccosine of x, in radians.|
|asin|(num:number):number |Returns the arcsine of x, in radians.|
|atan|(num:number):number |Returns the arctangent of x as a numeric value between -PI/2 and PI/2 radians.|
|atan2|(x:number, y:number):number |Returns the arctangent of the quotient of its arguments.|
|ceil|(num:number):number |Returns x, rounded upwards to the nearest integer.|
|cos|(num:number):number |Returns the cosine of x (x is in radians).|
|exp|(num:number):number |Returns the value of Ex.|
|floor|(num:number):number |Returns x, rounded downwards to the nearest integer.|
|log|(num:number):number |Returns the natural logarithm (base E) of x.|
|max|(x:number, y:number, ...):number |Returns the number with the highest value.|
|min|(x:number, y:number, ...):number |Returns the number with the lowest value.|
|pow|(x:number, y:number):number |Returns the value of x to the power of y.|
|random|():number |Returns a random number between 0 and 1.|
|round|(num:number):number |Rounds x to the nearest integer.|
|sin|(num:number):number |Returns the sine of x (x is in radians).|
|sqrt|(num:number):number |Returns the square root of x.|
|srand|(seed:number):number |Set random seed.|
|tan|(num:number):number |Returns the tangent of an angle.|


## MySql



Commands for accessing mysql databases.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|MySql|([options](#options-newmysql):object=void):userobj |Create a new db connection to a MySql database:.|
|affectedRows|():number |Return affected rows.|
|complete|(sql:string):boolean |Return true if sql is complete.|
|conf|([options](#options-mysqlconf):string&#124;object=void) |Configure options.|
|errorNo|():number |Return error code returned by most recent call to mysql3_exec().|
|errorState|():string |Return the mysql error state str.|
|eval|(sql:string):number |Run sql commands without input/output.|
|exists|(sql:string):boolean |Execute sql, and return true if there is at least one result value.|
|info|():object |Return info about last query.|
|lastQuery|():string |Return info string about most recently executed statement.|
|lastRowid|():number |Return rowid of last insert.|
|onecolumn|(sql:string) |Execute sql, and return a single value.|
|ping|(noError:boolean=false):number |Ping connection.|
|query|([options](#options-mysqlquery):function&#124;string&#124;array&#124;object=void) |Run sql query with input and/or outputs..|
|reconnect|():void |Reconnect with current settings.|
|reset|():number |Reset connection.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|bindWarn|*BOOL*|Treat failed variable binds as a warning.|initOnly|
|database|*STRKEY*|Database to use.|initOnly|
|debug|*ARRAY*|Enable debug trace for various operations. (zero or more of: **eval**, **delete**, **prepare**, **step**)||
|enableMulti|*BOOL*|Accept muiltiple semi-colon separated statements in eval().|initOnly|
|errorCnt|*INT*|Count of errors.|readOnly|
|queryOpts|*[Options](#options-queryOpts)*|Default options for exec.||
|forceInt|*BOOL*|Bind float as int if possible.||
|host|*STRING*|IP address or host name for mysqld (default is 127.0.0.1).||
|maxStmts|*INT*|Max cache size for compiled statements.||
|name|*DSTRING*|Name for this db handle.||
|numStmts|*INT*|Current size of compiled statement cache.|readOnly|
|password|*STRKEY*|Database password..|initOnly|
|port|*INT*|IP port for mysqld.|initOnly|
|reconnect|*BOOL*|Reconnect.||
|sslKey|*STRING*|SSL key.||
|sslCert|*STRING*|SSL Cert.||
|sslCA|*STRING*|SSL CA.||
|sslCAPath|*STRING*|SSL CA path.||
|sslCipher|*STRING*|SSL Cipher.||
|udata|*OBJ*|User data..||
|user|*STRKEY*|Database user name. Default is current user-name..|initOnly|
|version|*DOUBLE*|Mysql version number.|readOnly|
### Options "queryOpts"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function to call with each row result. @`function(values:object)`||
|headers|*BOOL*|First row returned contains column labels.||
|limit|*INT*|Maximum number of returned values.||
|mapundef|*BOOL*|In variable binds, map an 'undefined' var to null.||
|maxString|*INT*|If not using prefetch, the maximum string value size (0=8K).||
|mode|*STRKEY*|Set output mode of returned data. (one of: **rows**, **arrays**, **array1d**, **list**, **column**, **json**, **json2**, **html**, **csv**, **insert**, **line**, **tabs**, **none**)||
|nocache|*BOOL*|Disable query cache.||
|noNamedParams|*BOOL*|Disable translating sql to support named params.||
|nullvalue|*STRKEY*|Null string output (for non-json mode).||
|obj|*[Options](#options-obj)*|Options for object.||
|paramVar|*ARRAY*|Array var to use for parameters.||
|prefetch|*BOOL*|Let client library cache entire results.||
|separator|*STRKEY*|Separator string (for csv and text mode).||
|table|*STRKEY*|Table name for mode=insert.||
|typeCheck|*STRKEY*|Type check mode (error). (one of: **convert**, **error**, **warn**, **disable**)||
|values|*ARRAY*|Values for ? bind parameters.||
|width|*CUSTOM*|In column mode, set column widths.||
### Options "MySql.query"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function to call with each row result. @`function(values:object)`||
|headers|*BOOL*|First row returned contains column labels.||
|limit|*INT*|Maximum number of returned values.||
|mapundef|*BOOL*|In variable binds, map an 'undefined' var to null.||
|maxString|*INT*|If not using prefetch, the maximum string value size (0=8K).||
|mode|*STRKEY*|Set output mode of returned data. (one of: **rows**, **arrays**, **array1d**, **list**, **column**, **json**, **json2**, **html**, **csv**, **insert**, **line**, **tabs**, **none**)||
|nocache|*BOOL*|Disable query cache.||
|noNamedParams|*BOOL*|Disable translating sql to support named params.||
|nullvalue|*STRKEY*|Null string output (for non-json mode).||
|obj|*[Options](#options-obj)*|Options for object.||
|paramVar|*ARRAY*|Array var to use for parameters.||
|prefetch|*BOOL*|Let client library cache entire results.||
|separator|*STRKEY*|Separator string (for csv and text mode).||
|table|*STRKEY*|Table name for mode=insert.||
|typeCheck|*STRKEY*|Type check mode (error). (one of: **convert**, **error**, **warn**, **disable**)||
|values|*ARRAY*|Values for ? bind parameters.||
|width|*CUSTOM*|In column mode, set column widths.||
### Options "obj"
|Option|Type|Description|Flags|
|---|---|---|---|
|name|*STRKEY*|Name of object var data source for %s.||
|skip|*ARRAY*|Object members to ignore.||
|getSql|*BOOL*|Return expanded SQL without evaluating.||
|defaultNull|*BOOL*|Create with DEFAULT NULL.||
|noChecks|*BOOL*|Create with no CHECK constraints.||
|noDefaults|*BOOL*|Create with no defaults.||
|noTypes|*BOOL*|Create with no types.||


## Number



Commands for accessing number objects.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Number|(num:string=0):number |Number constructor.|
|isFinite|():boolean |Return true if is finite.|
|isInteger|():boolean |Return true if is an integer.|
|isNaN|():boolean |Return true if is NaN.|
|isSafeInteger|():boolean |Return true if is a safe integer.|
|toExponential|(num:number):string |Converts a number into an exponential notation.|
|toFixed|(num:number=0):string |Formats a number with x numbers of digits after the decimal point.|
|toPrecision|(num:number):string |Formats a number to x length.|
|toString|(radix:number=10):string |Convert to string.|


## Object



Commands for accessing Objects.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Object|(val:object&#124;function&#124;null=void):object |Object constructor.|
|assign|(obj:object,...):object |Return arg1 object with assigned values.|
|create|(proto:null&#124;object, properties:object=void):object |Create a new object with prototype object and properties.|
|freeze|(obj:object, freeze:boolean=true, modifyok:boolean=true, readcheck:boolean=true):void |Freeze/unfreeze an object with optionally.|
|getPrototypeOf|(name:object&#124;function):function&#124;object |Return prototype of an object.|
|hasOwnProperty|(name:string):boolean |Returns a true if object has the specified property.|
|is|(value1, value2):boolean |Tests if two values are equal.|
|isPrototypeOf|(name):boolean |Tests for an object in another object's prototype chain.|
|keys|(obj:object&#124;function=void):array |Return the keys of an object or array.|
|merge|(obj:object&#124;function):object |Return new object containing merged values.|
|propertyIsEnumerable|(name):boolean |Determine if a property is enumerable.|
|setPrototypeOf|(name:object, value:object) |Set prototype of an object.|
|toLocaleString|(quote:boolean=false):string |Convert to string.|
|toString|(quote:boolean=false):string |Convert to string.|
|valueOf|() |Returns primitive value.|
|values|(obj:object=void):array |Return the  values of an object.|


## RegExp



Commands for managing reqular expression objects.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|RegExp|(val:regexp&#124;string, flags:string):regexp |Create a regexp object.|
|exec|(val:string):array&#124;object&#124;null |return matching string. Perform regexp match checking.  Returns the array of matches.With the global flag g, sets lastIndex and returns next match.|
|test|(val:string):boolean |test if a string matches.|


## Signal



Commands for handling unix signals.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|alarm|(secs):number |Setup alarm in seconds.|
|callback|(func:function, sig:number&#124;string):number |Setup callback handler for signal.|
|handle|(sig:number&#124;string=void, ...) |Set named signals to handle action.|
|ignore|(sig:number&#124;string=void, ...) |Set named signals to ignore action.|
|kill|(pid:number, sig:number&#124;string='SIGTERM'):void |Send signal to process id.|
|names|():array |Return names of all signals.|
|reset|(sig:number&#124;string=void, ...):array |Set named signals to default action.|


## Socket



Commands for managing Socket server/client connections.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Socket|([options](#options-newsocket):object=void):userobj |Create socket server/client object.Create a socket server or client object.|
|close|():void |Close socket(s).|
|conf|([options](#options-socketconf):string&#124;object=void) |Configure options.|
|idconf|([options](#options-socketidconf):string&#124;object=void) |Configure options for a connection id, or return list of ids.|
|names|():array |Return list of active ids on server.|
|recv|(id:number=void):string |Recieve data.|
|send|([options](#options-socketsend):object=void):void |Send a socket message to id. Send a message to a (or all if -1) connection.|
|update|():void |Service events for just this socket.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|address|*STRING*|Client destination address (127.0.0.0).|initOnly|
|broadcast|*BOOL*|Enable broadcast.|initOnly|
|client|*BOOL*|Enable client mode.|initOnly|
|connectCnt|*INT*|Counter for number of active connections.|readOnly|
|createLast|*TIME_T*|Time of last create.|readOnly|
|debug|*INT*|Debugging level.||
|echo|*BOOL*|LogInfo outputs all socket Send/Recv messages.||
|interface|*STRING*|Interface for server to listen on, eg. 'eth0' or 'lo'.|initOnly|
|keepalive|*BOOL*|Enable keepalive.|initOnly|
|maxConnects|*INT*|In server mode, max number of client connections accepted.||
|mcastAddMember|*STRING*|Multicast add membership: address/interface ('127.0.0.1/0.0.0.0').|initOnly|
|mcastInterface|*STRING*|Multicast interface address.|initOnly|
|mcastNoLoop|*BOOL*|Multicast loopback disable.|initOnly|
|mcastTtl|*INT*|Multicast TTL.|initOnly|
|noAsync|*BOOL*|Send is not async.|initOnly|
|noUpdate|*BOOL*|Stop processing update events (eg. to exit).||
|onClose|*FUNC*|Function to call when connection closes. @`function(s:userobj|null, id:number)`||
|onCloseLast|*FUNC*|Function to call when last connection closes. On object delete arg is null. @`function(s:userobj|null)`||
|noConfig|*BOOL*|Disable use of Socket.conf to change options after create.|initOnly|
|onOpen|*FUNC*|Function to call when connection opens. @`function(s:userobj, info:object)`||
|onRecv|*FUNC*|Function to call with recieved data. @`function(s:userobj, id:number, data:string)`||
|port|*INT*|Port for client dest or server listen.|initOnly|
|quiet|*BOOL*|Suppress info messages.|initOnly|
|recvTimeout|*UINT64*|Timeout for receive, in microseconds.|initOnly|
|sendTimeout|*UINT64*|Timeout for send, in microseconds.|initOnly|
|srcAddress|*STRING*|Client source address.|initOnly|
|srcPort|*INT*|Client source port.|initOnly|
|startTime|*TIME_T*|Time of start.|readOnly|
|stats|*[Options](#options-stats)*|Statistical data.|readOnly|
|timeout|*NUMBER*|Timeout value in seconds (0.5).|initOnly|
|tos|*INT8*|Type-Of-Service value.|initOnly|
|ttl|*INT*|Time-To-Live value.|initOnly|
|udata|*OBJ*|User data.||
|udp|*BOOL*|Protocol is udp.|initOnly|
### Options "stats"
|Option|Type|Description|Flags|
|---|---|---|---|
|echo|*BOOL*|LogInfo outputs all socket Send/Recv messages.||
|eventCnt|*INT*|Number of events of any type.||
|eventLast|*TIME_T*|Time of last event of any type.||
|recvAddr|*CUSTOM*|Incoming port and address.||
|recvCnt|*INT*|Number of recieves.||
|recvLast|*TIME_T*|Time of last recv.||
|sentCnt|*INT*|Number of sends.||
|sentLast|*TIME_T*|Time of last send.||
|sentErrCnt|*INT*|Number of sends.||
|sentErrLast|*TIME_T*|Time of last sendErr.||
|udata|*OBJ*|User data.||
### Options "Socket.idconf"
|Option|Type|Description|Flags|
|---|---|---|---|
|echo|*BOOL*|LogInfo outputs all socket Send/Recv messages.||
|eventCnt|*INT*|Number of events of any type.||
|eventLast|*TIME_T*|Time of last event of any type.||
|recvAddr|*CUSTOM*|Incoming port and address.||
|recvCnt|*INT*|Number of recieves.||
|recvLast|*TIME_T*|Time of last recv.||
|sentCnt|*INT*|Number of sends.||
|sentLast|*TIME_T*|Time of last send.||
|sentErrCnt|*INT*|Number of sends.||
|sentErrLast|*TIME_T*|Time of last sendErr.||
|udata|*OBJ*|User data.||
### Options "Socket.send"
|Option|Type|Description|Flags|
|---|---|---|---|
|noAsync|*BOOL*|Send is not async.||


## Sqlite



Commands for accessing sqlite databases.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|Sqlite|([options](#options-newsqlite):object=void):userobj |Create a new db connection to the named file or :memory:.|
|backup|(file:string, dbname:string='main'):void |Backup db to file. Open or create a database file named FILENAME. Transfer the content of local database DATABASE (default: 'main') into the FILENAME database.|
|collate|(name:string, callback:function):void |Create new SQL collation command.|
|complete|(sql:string):boolean |Return true if sql is complete.|
|conf|([options](#options-sqliteconf):string&#124;object=void) |Configure options.|
|eval|(sql:string):number |Run sql commands without input/output. Supports multiple semicolon seperated commands. Variable binding is NOT performed, results are discarded, and  returns sqlite3_changes()|
|exists|(sql:string):boolean |Execute sql, and return true if there is at least one result value.|
|filename|(name:string='main'):string |Return filename for named or all attached databases.|
|func|(name:string, callback:function, numArgs:number=void):void |Register a new function with database.|
|import|([options](#options-sqliteimport):object=void):number |Import data from file into table . Import data from a file into table. SqlOptions include the 'separator' to use, which defaults to commas for csv, or tabs otherwise. If a column contains a null string, or the value of 'nullvalue', a null is inserted for the column. A 'conflict' is one of the sqlite conflict algorithms:    rollback, abort, fail, ignore, replace On success, return the number of lines processed, not necessarily same as 'changeCnt' due to the conflict algorithm selected. |
|interrupt|():void |Interrupt in progress statement.|
|onecolumn|(sql:string) |Execute sql, and return a single value.|
|query|([options](#options-sqlitequery):function&#124;string&#124;array&#124;object=void) |Evaluate an sql query with bindings. Return values in formatted as JSON, HTML, etc. , optionally calling function with a result object|
|restore|(file:string, dbname:string):void |Restore db from file (default db is 'main').    db.restore(FILENAME, ?,DATABASE? )  Open a database file named FILENAME.  Transfer the content of FILENAME into the local database DATABASE (default: 'main').|
|transaction|(callback:function, type:string=void):void |Call function inside db tranasaction. Type is: 'deferred', 'exclusive', 'immediate'. Start a new transaction (if we are not already in the midst of a transaction) and execute the JS function FUNC. After FUNC completes, either commit the transaction or roll it back if FUNC throws an exception. Or if no new transation was started, do nothing. pass the exception on up the stack.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|bindWarn|*BOOL*|Treat failed variable binds as a warning.|initOnly|
|changeCnt|*INT*|The number of rows modified, inserted, or deleted by last command.||
|changeCntAll|*INT*|Total number of rows modified, inserted, or deleted since db opened.||
|debug|*ARRAY*|Enable debug trace for various operations. (zero or more of: **eval**, **delete**, **prepare**, **step**)||
|echo|*BOOL*|Output query/eval string to log.||
|errCnt|*INT*|Count of errors in script callbacks.|readOnly|
|errorCode|*INT*|Numeric error code returned by the most recent call to sqlite3_exec.||
|forceInt|*BOOL*|Bind float as int if possible.||
|noJsonConv|*BOOL*|Do not JSON auto-convert array and object in CHARJSON columns.||
|lastInsertId|*UINT64*|The rowid of last insert.||
|load|*BOOL*|Extensions can be loaded.||
|maxRegexCache|*INT*|Max cache size for regex patterns; 0=disable, -1=unlimited (100).||
|mutex|*STRKEY*|Mutex type to use. (one of: **default**, **none**, **full**)|initOnly|
|name|*DSTRING*|The dbname to use instead of 'main'.|initOnly|
|noConfig|*BOOL*|Disable use of Sqlite.conf to change options after create.|initOnly|
|noCreate|*BOOL*|Database is must already exist (false).|initOnly|
|onAuth|*FUNC*|Function to call for auth. @`function(db:userobj, code:string, descr1:string, decr2:string, dbname:string, trigname:string)`||
|onBusy|*FUNC*|Function to call when busy. @`function(db:userobj, tries:number)`||
|onCommit|*FUNC*|Function to call on commit. @`function(db:userobj)`||
|onNeedCollate|*FUNC*|Function to call for collation. @`function(db:userobj, name:string)`||
|onProfile|*FUNC*|Function to call for profile. @`function(db:userobj, sql:string, time:number)`||
|onProgress|*FUNC*|Function to call for progress: progressSteps must be >0. @`function(db:userobj)`||
|onRollback|*FUNC*|Function to call for rollback. @`function(db:userobj)`||
|onTrace|*FUNC*|Function to call for trace. @`function(db:userobj, sql:string)`||
|onUpdate|*FUNC*|Function to call for update. @`function(db:userobj, op:string, dbname:string, table:string, rowid:number)`||
|onWalHook|*FUNC*|Function to call for WAL. @`function(db:userobj, dbname:string, entry:number)`||
|progressSteps|*UINT*|Number of steps between calling onProgress: 0 is disabled.||
|queryOpts|*[Options](#options-queryOpts)*|Default options for to use with query().||
|readonly|*BOOL*|Database opened in readonly mode.|initOnly|
|sortCnt|*INT*|Number of sorts in most recent operation.|readOnly|
|stepCnt|*INT*|Number of steps in most recent operation.|readOnly|
|stmtCacheCnt|*INT*|Current size of compiled statement cache.|readOnly|
|stmtCacheMax|*INT*|Max cache size for compiled statements.||
|timeout|*INT*|Amount of time to wait when file is locked, in ms.||
|udata|*OBJ*|User data.||
|version|*OBJ*|Sqlite version info.|readOnly|
|timeout|*INT*|Amount of time to wait when file is locked, in ms.||
|vfs|*STRING*|VFS to use.|initOnly|
### Options "queryOpts"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function to call with each row result. @`function(values:object)`||
|cdata|*STRKEY*|Name of Cdata array object to use.||
|echo|*BOOL*|Output query string to log.||
|headers|*BOOL*|First row returned contains column labels.||
|limit|*INT*|Maximum number of returned values.||
|mapundef|*BOOL*|In variable bind, map an 'undefined' var to null.||
|mode|*STRKEY*|Set output mode of returned data. (one of: **rows**, **arrays**, **array1d**, **list**, **column**, **json**, **json2**, **html**, **csv**, **insert**, **line**, **tabs**, **none**)||
|nocache|*BOOL*|Disable query cache.||
|nullvalue|*STRKEY*|Null string output (for non js/json mode).||
|obj|*[Options](#options-obj)*|Options for object.||
|retChanged|*BOOL*|Query returns value of sqlite3_changed().||
|separator|*STRKEY*|Separator string (for csv and text mode).||
|typeCheck|*STRKEY*|Type check mode (warn). (one of: **convert**, **warn**, **error**, **disable**)||
|table|*STRKEY*|Table name for mode=insert.||
|values|*ARRAY*|Values for ? bind parameters.||
|width|*CUSTOM*|In column mode, set column widths.||
### Options "Sqlite.import"
|Option|Type|Description|Flags|
|---|---|---|---|
|headers|*BOOL*|First row contains column labels.||
|csv|*BOOL*|Treat input values as CSV.||
|conflict|*STRKEY*|Set conflict resolution. (one of: **ROLLBACK**, **ABORT**, **FAIL**, **IGNORE**, **REPLACE**)||
|limit|*INT*|Maximum number of lines to load.||
|nullvalue|*STRKEY*|Null string.||
|separator|*STRKEY*|Separator string; default is comma if csv, else tabs.||
### Options "Sqlite.query"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function to call with each row result. @`function(values:object)`||
|cdata|*STRKEY*|Name of Cdata array object to use.||
|echo|*BOOL*|Output query string to log.||
|headers|*BOOL*|First row returned contains column labels.||
|limit|*INT*|Maximum number of returned values.||
|mapundef|*BOOL*|In variable bind, map an 'undefined' var to null.||
|mode|*STRKEY*|Set output mode of returned data. (one of: **rows**, **arrays**, **array1d**, **list**, **column**, **json**, **json2**, **html**, **csv**, **insert**, **line**, **tabs**, **none**)||
|nocache|*BOOL*|Disable query cache.||
|nullvalue|*STRKEY*|Null string output (for non js/json mode).||
|obj|*[Options](#options-obj)*|Options for object.||
|retChanged|*BOOL*|Query returns value of sqlite3_changed().||
|separator|*STRKEY*|Separator string (for csv and text mode).||
|typeCheck|*STRKEY*|Type check mode (warn). (one of: **convert**, **warn**, **error**, **disable**)||
|table|*STRKEY*|Table name for mode=insert.||
|values|*ARRAY*|Values for ? bind parameters.||
|width|*CUSTOM*|In column mode, set column widths.||


## String



Commands for accessing string objects..


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|String|(str):string |String constructor.|
|charAt|(index:number):string |Return char at index.|
|charCodeAt|(index:number):number |Return char code at index.|
|concat|(str:string, ...):string |Append one or more strings.|
|fromCharCode|(...):string |Return string for char codes.|
|indexOf|(str:string, start:number):number |Return index of char.|
|lastIndexOf|(str:string, start:number):number |Return index of last char.|
|map|(strMap:array, nocase:boolean=false):string |Replaces characters in string based on the key-value pairs in strMap.|
|match|(pattern:regexp&#124;string):array&#124;null |Return array of matches.|
|repeat|(count:number):string |Return count copies of string.|
|replace|(pattern:regexp&#124;string, replace:string&#124;function):string |Regex/string replacement. If the replace argument is a function, it is called with match,p1,p2,...,offset,string.  If called function is known to have 1 argument, it is called with just the match.|
|search|(pattern:regexp&#124;string):number |Return index of first char matching pattern.|
|slice|(start:number, end:number):string |Return section of string.|
|split|(char:string&#124;null=void):array |Split on char and return Array. When char is omitted splits on bytes.  When char==null splits on whitespace and removes empty elements.|
|substr|(start:number, length:number):string |Return substring.|
|substring|(start:number, end:number):string |Return substring.|
|toLocaleLowerCase|():string |Lower case.|
|toLocaleUpperCase|():string |Upper case.|
|toLowerCase|():string |Return lower cased string.|
|toTitle|(chars:string):string |Make first char upper case.|
|toUpperCase|():string |Return upper cased string.|
|trim|(chars:string):string |Trim chars.|
|trimLeft|(chars:string):string |Trim chars from left.|
|trimRight|(chars:string):string |Trim chars from right.|


## System



Builtin system commands. All methods are exported as global.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|LogDebug|(str:string&#124;boolean,...):void |Debug logging command.|
|LogError|(str:string&#124;boolean,...):void |Debug logging command.|
|LogInfo|(str:string&#124;boolean,...):void |Debug logging command.|
|LogTest|(str:string&#124;boolean,...):void |Debug logging command.|
|LogTrace|(str:string&#124;boolean,...):void |Debug logging command.|
|LogWarn|(str:string&#124;boolean,...):void |Debug logging command.|
|assert|([options](#options-systemassert):object=void):void |Throw or output msg if expr is false. Assertions.  Enable with jsish --I Assert or using the -Assert module option.|
|clearInterval|(id:number):void |Delete event id returned from setInterval/setTimeout/info.events().|
|decodeURI|(val:string):string |Decode an HTTP URL.|
|encodeURI|(val:string):string |Encode an HTTP URL.|
|exec|([options](#options-systemexec):string&#124;object=void) |Execute an OS command. If the command ends with '&', set the 'bg' option to true. The second argument can be a string, which is the same as setting the 'inputStr' option. By default, returns the string output, unless the 'bg', 'inputStr', 'retCode' or 'retAll' options are used|
|exit|(code:number=0):void |Exit the current interpreter.|
|format|(format:string, ...):string |Printf style formatting: adds %q and %S.|
|import|([options](#options-systemimport):object=void) |Same as source with {import:true}.|
|isFinite|(val):boolean |Return true if is a finite number.|
|isMain|():boolean |Return true if current script was the main script invoked from command-line.|
|isNaN|(val):boolean |Return true if not a number.|
|load|(shlib:string):void |Load a shared executable and invoke its _Init call.|
|log|(val, ...):void |Same as puts, but includes file:line.|
|matchObj|(obj:object, match:string=void, partial=false, noerror=false):string&#124;boolean |Validate that object matches given name:type string. With single arg returns generated string.|
|moduleOpts|(options:object=void, self:object&#124;userobj=void, conf:object&#124;null&#124;undefined=void):object |Parse module options.|
|moduleRun|(cmd:string&#124;null&#124;function=void, conf:array=undefined) |Invoke named module. If name is empty, uses file basename. If isMain invokes function with same name as file. With no args will invoke provide.|
|noOp|() |A No-Op. A zero overhead command call that is useful for debugging.|
|parseFloat|(val):number |Convert string to a double.|
|parseInt|(val:any, base:number=10):number |Convert string to an integer.|
|parseOpts|(self:object&#124;userobj, options:object, conf:object&#124;null&#124;undefined=void):object |Parse module options: similar to moduleOpts but args are different.|
|printf|(format:string, ...):void |Formatted output to stdout.|
|provide|([options](#options-systemprovide):object&#124;function=void):void |Provide a package for use with require.. Default is the file tail-rootname|
|puts|(val, ...):void |Output one or more values to stdout. Each argument is quoted.  Use Interp.logOpts to control source line and/or timestamps output.|
|quote|(val:string):string |Return quoted string.|
|require|([options](#options-systemrequire):object=void):number&#124;array&#124;object |Load/query packages. With no arguments, returns the list of all loaded packages. With one argument, loads the package (if necessary) and returns its version. With two arguments, returns object containing: version, loadFile, func. A third argument sets options for package or module. Note an error is thrown if requested version is greater than actual version.|
|runModule|(cmd:string&#124;null&#124;function=void, conf:array=undefined) |Invoke named module. If name is empty, uses file basename. If isMain invokes function with same name as file. With no args will invoke provide.|
|setInterval|(callback:function, ms:number):number |Setup recurring function to run every given millisecs.|
|setTimeout|(callback:function, ms:number):number |Setup function to run after given millisecs.|
|sleep|(secs:number=1.0):void |sleep for N milliseconds, minimum .001.|
|source|([options](#options-systemsource):object=void) |Load and evaluate source files: trailing '/' appends PARENTDIR.jsi.|
|strftime|([options](#options-systemstrftime):string&#124;object=void):string |Format numeric time (in ms) to a string. Null or no value will use current time.|
|strptime|([options](#options-systemstrptime):string&#124;object=void):number |Parse time from string and return ms time since 1970-01-01 in UTC, or NaN.|
|times|(callback:function&#124;boolean, count:number=1):number |Call function count times and return execution time in microseconds.|
|unload|(shlib:string):void |Unload a shared executable and invoke its _Done call.|
|update|([options](#options-systemupdate):number&#124;object=void):number |Service all events, eg. setInterval/setTimeout. Returns the number of events processed. Events are processed until minTime (in milliseconds) is exceeded, or forever if -1. The default minTime is 0, meaning return as soon as no events can be processed. A positive mintime will result in sleeps between event checks.|
### Options "System.assert"
|Option|Type|Description|Flags|
|---|---|---|---|
|mode|*STRKEY*|Action when assertion fails. Default from Interp.assertMode. (one of: **throw**, **log**, **puts**)||
|noStderr|*BOOL*|Logged msg to stdout. Default from Interp.noStderr.||
### Options "System.exec"
|Option|Type|Description|Flags|
|---|---|---|---|
|bg|*BOOL*|Run command in background using system() and return OS code.||
|chdir|*STRING*|Change to directory.||
|inputStr|*STRING*|Use string as input and return OS code.||
|noError|*BOOL*|Suppress all OS errors.||
|noRedir|*BOOL*|Disable redirect and shell escapes in command.||
|noShell|*BOOL*|Do not use native popen which invokes via /bin/sh.||
|trim|*BOOL*|Trim trailing whitespace from output.||
|retAll|*BOOL*|Return the OS return code and data as an object.||
|retCode|*BOOL*|Return only the OS return code.||
### Options "System.import"
|Option|Type|Description|Flags|
|---|---|---|---|
|autoIndex|*BOOL*|Look for and load Jsi_Auto.jsi auto-index file.||
|exists|*BOOL*|Source file only if exists.||
|global|*BOOL*|File is to be sourced in global frame rather than local.||
|import|*BOOL*|Wrap file contents in a return/function closure.||
|isMain|*BOOL*|Coerce to true the value of Info.isMain().||
|level|*UINT*|Frame to source file in.||
|noEval|*BOOL*|Disable eval: just parses file to check syntax.||
|noError|*BOOL*|Ignore errors in sourced file.||
|once|*BOOL*|Source file only if not already sourced (Default: Interp.debugOpts.includeOnce).||
|trace|*BOOL*|Trace include statements (Default: Interp.debugOpts.includeTrace).||
### Options "System.provide"
|Option|Type|Description|Flags|
|---|---|---|---|
|log|*ARRAY*|Logging flags. (zero or more of: **bug**, **assert**, **debug**, **trace**, **test**, **info**, **warn**, **error**, **parse**)|noCase|
|logmask|*ARRAY*|Logging mask flags. (zero or more of: **bug**, **assert**, **debug**, **trace**, **test**, **info**, **warn**, **error**, **parse**)|noCase|
|coverage|*BOOL*|On exit generate detailed code coverage for function calls (with profile).||
|nofreeze|*BOOL*|Disable moduleOpts freeze of first arg (self).||
|info|*OBJ*|Info provided by module.|initOnly|
|profile|*BOOL*|On exit generate profile of function calls.||
|traceCall|*ARRAY*|Trace commands. (zero or more of: **funcs**, **cmds**, **new**, **return**, **args**, **notrunc**, **noparent**, **full**, **before**)||
|udata|*OBJ*|User data settable by require.||
### Options "System.require"
|Option|Type|Description|Flags|
|---|---|---|---|
|log|*ARRAY*|Logging flags. (zero or more of: **bug**, **assert**, **debug**, **trace**, **test**, **info**, **warn**, **error**, **parse**)|noCase|
|logmask|*ARRAY*|Logging mask flags. (zero or more of: **bug**, **assert**, **debug**, **trace**, **test**, **info**, **warn**, **error**, **parse**)|noCase|
|coverage|*BOOL*|On exit generate detailed code coverage for function calls (with profile).||
|nofreeze|*BOOL*|Disable moduleOpts freeze of first arg (self).||
|info|*OBJ*|Info provided by module.|initOnly|
|profile|*BOOL*|On exit generate profile of function calls.||
|traceCall|*ARRAY*|Trace commands. (zero or more of: **funcs**, **cmds**, **new**, **return**, **args**, **notrunc**, **noparent**, **full**, **before**)||
|udata|*OBJ*|User data settable by require.||
### Options "System.source"
|Option|Type|Description|Flags|
|---|---|---|---|
|autoIndex|*BOOL*|Look for and load Jsi_Auto.jsi auto-index file.||
|exists|*BOOL*|Source file only if exists.||
|global|*BOOL*|File is to be sourced in global frame rather than local.||
|import|*BOOL*|Wrap file contents in a return/function closure.||
|isMain|*BOOL*|Coerce to true the value of Info.isMain().||
|level|*UINT*|Frame to source file in.||
|noEval|*BOOL*|Disable eval: just parses file to check syntax.||
|noError|*BOOL*|Ignore errors in sourced file.||
|once|*BOOL*|Source file only if not already sourced (Default: Interp.debugOpts.includeOnce).||
|trace|*BOOL*|Trace include statements (Default: Interp.debugOpts.includeTrace).||
### Options "System.strftime"
|Option|Type|Description|Flags|
|---|---|---|---|
|secs|*BOOL*|Time is seconds (out for parse, in for format).||
|fmt|*STRKEY*|Format string for time.||
|iso|*BOOL*|ISO fmt plus milliseconds ie: %FT%T.%f.||
|utc|*BOOL*|Time is utc (in for parse, out for format).||
### Options "System.strptime"
|Option|Type|Description|Flags|
|---|---|---|---|
|secs|*BOOL*|Time is seconds (out for parse, in for format).||
|fmt|*STRKEY*|Format string for time.||
|iso|*BOOL*|ISO fmt plus milliseconds ie: %FT%T.%f.||
|utc|*BOOL*|Time is utc (in for parse, out for format).||
### Options "System.update"
|Option|Type|Description|Flags|
|---|---|---|---|
|maxEvents|*INT*|Maximum number of events to process (or -1 for all).||
|maxPasses|*INT*|Maximum passes through event queue.||
|minTime|*INT*|Minimum milliseconds before returning, or -1 to loop forever (default is 0).||
|sleep|*INT*|Time to sleep time (in milliseconds) between event checks. Default is 1.||


## Util



Utilities commands.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|argArray|(arg:any&#124;undefined):array&#124;null |Coerces non-null to an array, if necessary.|
|base64|(val:string, decode:boolean=false):string |Base64 encode/decode a string.|
|complete|(val:string):boolean |Return true if string is complete command with balanced braces, etc.|
|crc32|(val:string, crcSeed=0):number |Calculate 32-bit CRC.|
|decrypt|(val:string, key:string):string |Decrypt data using BTEA encryption. Keys that are not 16 bytes use the MD5 hash of the key.|
|encrypt|(val:string, key:string):string |Encrypt data using BTEA encryption. Keys that are not 16 bytes use the MD5 hash of the key.|
|fromCharCode|(code:number):string |Return char with given character code.|
|getenv|(name:string=void):string&#124;object&#124;void |Get one or all environment.|
|getpid|(parent:boolean=false):number |Get process/parent id.|
|getuser|():object |Get userid info.|
|hash|([options](#options-utilhash):object=void):string |Return hash (default SHA256) of string/file.|
|hexStr|(val:string, decode:boolean=false):string |Hex encode/decode a string.|
|setenv|(name:string, value:string=void) |Set/get an environment var.|
|sqlValues|(name:string, obj:object=void) |Get object values for SQL.|
|times|(callback:function&#124;boolean, count:number=1):number |Call function count times and return execution time in microseconds.|
|verConvert|(ver:string&#124;number, zeroTrim:number=0):number&#124;string&#124;null |Convert a version to/from a string/number, or return null if not a version. For string output zeroTrim says how many trailing .0 to trim (0-2).|
### Options "Util.hash"
|Option|Type|Description|Flags|
|---|---|---|---|
|file|*STRING*|Read data from file and append to str.||
|hashcash|*UINT*|Search for a hash with this many leading zero bits by appending :nonce (Proof-Of-Work).||
|noHex|*BOOL*|Return binary digest, without conversion to hex chars.||
|type|*STRKEY*|Type of hash. (one of: **sha256**, **sha1**, **md5**, **sha3_224**, **sha3_384**, **sha3_512**, **sha3_256**)||


## Vfs



Commands for creating in memory readonly Virtual file-systems.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|conf|([options](#options-vfsconf):string&#124;object&#124;string=void) |Configure mount.|
|exec|(cmd:string) |Safe mode exec for VFS support cmds eg. fossil info/ls/cat.|
|fileconf|([options](#options-vfsfileconf):string&#124;object=void) |Configure file info which is same info as in fileList.|
|list|():array |Return list of all vfs mounts.|
|mount|(type:string, file:string, param:object=void):string |Mount fossil file as given VFS type name, returning the mount point: frontend for vmount.|
|type|([options](#options-vfstype):object&#124;null=void) |Set/get/delete VFS type name.|
|unmount|(mount:string):void |Unmount a VFS.|
|vmount|([options](#options-vfsvmount):object=void):string |Create and mount a VFS, returning the mount point.|
### Options "Vfs.conf"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function implementing VFS. @`function(op:string, mount:string, arg:string|object|null)`||
|extra|*OBJ*|Extra info, typically used by predefined VFS type.||
|noAddDirs|*BOOL*|Disable auto-adding of directories; needed by File.glob.||
|file|*STRING*|Fossil file to mount.||
|fileList|*ARRAY*|List of files in the VFS (from listFunc).||
|info|*OBJ*|Info for VFS that is stored upon init.||
|mount|*STRING*|Mount point for the VFS.||
|noPatches|*BOOL*|Ignore patchlevel updates: accepts only X.Y releases.||
|param|*OBJ*|Optional 3rd argument passed to mount.||
|type|*STRKEY*|Type for predefined VFS.||
|user|*OBJ*|User data.||
|version|*STRKEY*|Version to mount.||
### Options "Vfs.exec"
|Option|Type|Description|Flags|
|---|---|---|---|
|data|*STRING*|Data for file.||
|file|*STRKEY*|File pathname.|required|
|perms|*UINT32*|Permissions for file.||
|size|*SSIZE_T*|Size of file.||
|timestamp|*TIME_T*|Timestamp of file.||
### Options "Vfs.fileconf"
|Option|Type|Description|Flags|
|---|---|---|---|
|data|*STRING*|Data for file.||
|file|*STRKEY*|File pathname.|required|
|perms|*UINT32*|Permissions for file.||
|size|*SSIZE_T*|Size of file.||
|timestamp|*TIME_T*|Timestamp of file.||
### Options "Vfs.type"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function implementing VFS. @`function(op:string, mount:string, arg:string|object|null)`|required|
|extra|*OBJ*|Extra info, typically used by predefined VFS type.||
|noAddDirs|*BOOL*|Disable auto-adding of directories; needed by File.glob.||
### Options "Vfs.vmount"
|Option|Type|Description|Flags|
|---|---|---|---|
|callback|*FUNC*|Function implementing VFS. @`function(op:string, mount:string, arg:string|object|null)`||
|extra|*OBJ*|Extra info, typically used by predefined VFS type.||
|noAddDirs|*BOOL*|Disable auto-adding of directories; needed by File.glob.||
|file|*STRING*|Fossil file to mount.||
|fileList|*ARRAY*|List of files in the VFS (from listFunc).||
|info|*OBJ*|Info for VFS that is stored upon init.||
|mount|*STRING*|Mount point for the VFS.||
|noPatches|*BOOL*|Ignore patchlevel updates: accepts only X.Y releases.||
|param|*OBJ*|Optional 3rd argument passed to mount.||
|type|*STRKEY*|Type for predefined VFS.||
|user|*OBJ*|User data.||
|version|*STRKEY*|Version to mount.||


## WebSocket



Commands for managing WebSocket server/client connections.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|WebSocket|([options](#options-newwebsocket):object=void):userobj |Create websocket server/client object.Create a websocket server/client object.  The server serves out pages to a web browser, which can use javascript to upgrade connection to a bidirectional websocket.|
|conf|([options](#options-websocketconf):string&#124;object=void) |Configure options.|
|file|(name:string=void):array&#124;void |Add file to hash, or with no args return file hash.|
|handler|(extension:string=void, cmd:string&#124;function=void, flags:number=0):string&#124;array&#124;function&#124;void |Get/Set handler command for an extension. With no args, returns list of handlers.  With one arg, returns value for that handler. Otherwise, sets the handler. When cmd is a string, the call is via moduleRun([cmd], arg). If a cmd is a function, it is called with a single arg: the file name.|
|header|(id:number, name:string=void):string&#124;array&#124;void |Get one or all input headers for connect id.|
|idconf|([options](#options-websocketidconf):string&#124;object=void) |Configure options for connect id.|
|ids|(name:string=void):array |Return list of ids, or lookup one id.|
|query|(id:number, name:string=void):string&#124;object&#124;void |Get one or all query values for connect id.|
|send|(id:number, data:any):void |Send a websocket message to id. Send a message to one (or all connections if -1). If not already a string, msg is formatted as JSON prior to the send.|
|status|():object&#124;void |Return liblws server status.|
|unalias|(path:string):string&#124;void |Return alias reverse lookup.|
|update|():void |Service events for just this websocket.|
|version|():string |Runtime library version string.|
### Options "new"
|Option|Type|Description|Flags|
|---|---|---|---|
|address|*STRING*|In client-mode the address to connect to (127.0.0.1).||
|bufferPwr2|*INT*|Tune the recv/send buffer: value is a power of 2 in [0-20] (16).||
|client|*BOOL*|Run in client mode.|initOnly|
|clientHost|*STRKEY*|Override host name for client.||
|clientOrigin|*STRKEY*|Override client origin (origin).||
|debug|*INT*|Set debug level. Setting this to 512 will turn on max liblws log levels.||
|echo|*BOOL*|LogInfo outputs all websock Send/Recv messages.||
|formParams|*STRKEY*|Comma seperated list of upload form param names ('text,send,file,upload').|readOnly|
|extHandlers|*BOOL*|Setup builtin extension-handlers, ie: .htmli, .cssi, .jsi, .mdi.|initOnly|
|extOpts|*OBJ*|Key/value store for extension-handlers options.|initOnly|
|flags|*INT*|Flags for future use..||
|getRegexp|*REGEXP*|Call onGet() only if Url matches pattern.||
|headers|*ARRAY*|Headers to send to browser: name/value pairs.|initOnly|
|jsiFnPattern|*STRKEY*|A glob-match pattern for files to which is appended 'window.jsiWebSocket=true;' (jsig*.js).|readOnly|
|interface|*STRING*|Interface for server to listen on, eg. 'eth0' or 'lo'.|initOnly|
|local|*BOOL*|Limit connections to localhost addresses on the 127 network.||
|localhostName|*STRKEY*|Client name used by localhost connections ('localhost').||
|maxConnects|*INT*|In server mode, max number of client connections accepted.||
|maxDownload|*INT*|Max size of file download.||
|maxUpload|*INT*|Max size of file upload will accept.||
|mimeTypes|*OBJ*|Object map of file-exts to mime types; initial and/or override of builtins.||
|mimeLookupFunc|*FUNC*|Function to call to lookup mime; returns and/or inserts into mimeTypes. @`function(ws:userobj, id:number, extension:string, url:string)`||
|modifySecs|*UINT*|Seconds between checking for modified files with onModify (2).||
|noConfig|*BOOL*|Disable use of conf() to change options after options after create.|initOnly|
|noCompress|*BOOL*|Disable per-message-deflate extension which can truncate large msgs.||
|noUpdate|*BOOL*|Disable update event-processing.||
|noWebsock|*BOOL*|Serve html, but disallow websocket upgrade.|initOnly|
|noWarn|*BOOL*|Quietly ignore file related errors.||
|onAuth|*FUNC*|Function to call for http basic authentication. @`function(ws:userobj, id:number, url:string, userpass:string)`||
|onClose|*FUNC*|Function to call when the websocket connection closes. @`function(ws:userobj|null, id:number, isError:boolean)`||
|onCloseLast|*FUNC*|Function to call when last websock connection closes. On object delete arg is null. @`function(ws:userobj|null)`||
|onFilter|*FUNC*|Function to call on a new connection: return false to kill connection. @`function(ws:userobj, id:number, url:string, ishttp:boolean)`||
|onGet|*FUNC*|Function to call to server handle http-get. @`function(ws:userobj, id:number, url:string, query:array)`||
|onModify|*FUNC*|Function to call when a served-out-file is modified. @`function(ws:userobj, file:string)`|initOnly|
|onOpen|*FUNC*|Function to call when the websocket connection occurs. @`function(ws:userobj, id:number)`||
|onUnknown|*FUNC*|Function to call to server out content when no file exists. @`function(ws:userobj, id:number, url:string, query:array)`||
|onUpload|*FUNC*|Function to call handle http-post. @`function(ws:userobj, id:number, filename:string, data:string, startpos:number, complete:boolean)`||
|onRecv|*FUNC*|Function to call when websock data recieved. @`function(ws:userobj, id:number, data:string)`||
|pathAliases|*OBJ*|Path alias lookups.|initOnly|
|port|*INT*|Port for server to listen on (8080).|initOnly|
|post|*STRING*|Post string to serve.|initOnly|
|protocol|*STRKEY*|Name of protocol (ws/wss).||
|realm|*STRKEY*|Realm for basic auth (jsish).||
|recvBufMax|*INT*|Size limit of a websocket message.|initOnly|
|recvBufTimeout|*INT*|Timeout for recv of a websock msg.|initOnly|
|redirMax|*BOOL*|Temporarily disable redirects when see more than this in 10 minutes.||
|rootdir|*STRING*|Directory to serve html from (".").||
|server|*STRKEY*|String to send out int the header SERVER (jsiWebSocket).||
|ssiExts|*OBJ*|Object map of file extensions to apply SSI.  eg. {myext:true, shtml:false} .|initOnly|
|ssl|*BOOL*|Use https.|initOnly|
|sslCert|*STRKEY*|SSL certificate file.||
|sslKey|*STRKEY*|SSL key file.||
|stats|*[Options](#options-stats)*|Statistical data.|readOnly|
|startTime|*TIME_T*|Time of websocket start.|readOnly|
|includeFile|*STRKEY*|Default file when no extension given (include.shtml).||
|udata|*OBJ*|User data.||
|urlPrefix|*STRKEY*|Prefix in url to strip from path; for reverse proxy..||
|urlRedirect|*STRKEY*|Redirect when no url or /, and adds cookie sessionJsi..||
|useridPass|*STRKEY*|The USERID:PASSWORD to use for basic authentication.||
|version|*OBJ*|WebSocket version info.|readOnly|
### Options "WebSocket.idconf"
|Option|Type|Description|Flags|
|---|---|---|---|
|clientIP|*STRKEY*|Client IP Address.|readOnly|
|clientName|*STRKEY*|Client hostname.|readOnly|
|echo|*BOOL*|LogInfo outputs all websock Send/Recv messages.||
|headers|*ARRAY*|Headers to send to browser on connection: name/value pairs.||
|isWebsock|*BOOL*|Is a websocket connection.||
|key|*STRBUF*|String key lookup in ids command for SSI echo ${#}.|readOnly|
|onClose|*FUNC*|Function to call when the websocket connection closes. @`function(ws:userobj|null, id:number, isError:boolean)`||
|onGet|*FUNC*|Function to call to server handle http-get. @`function(ws:userobj, id:number, url:string, query:array)`||
|onUnknown|*FUNC*|Function to call to serve out content when no file exists. @`function(ws:userobj, id:number, url:string, args:array)`||
|onRecv|*FUNC*|Function to call when websock data recieved. @`function(ws:userobj, id:number, data:string)`||
|onUpload|*FUNC*|Function to call handle http-post. @`function(ws:userobj, id:number, filename:string, data:string, startpos:number, complete:boolean)`||
|rootdir|*STRING*|Directory to serve html from (".").||
|stats|*[Options](#options-stats)*|Statistics for connection.|readOnly|
|query|*ARRAY*|Uri arg values for connection.||
|queryObj|*OBJ*|Uri arg values for connection as an object.||
|udata|*OBJ*|User data.||
|url|*DSTRING*|Url for connection.||
|username|*STRING*|The login userid for this connection.||
### Options "stats"
|Option|Type|Description|Flags|
|---|---|---|---|
|connectCnt|*INT*|Number of active connections.|readOnly|
|createTime|*TIME_T*|Time created.||
|eventCnt|*INT*|Number of events of any type.||
|eventLast|*TIME_T*|Time of last event of any type.||
|httpCnt|*INT*|Number of http reqs.||
|httpLast|*TIME_T*|Time of last http reqs.||
|isBinary|*BOOL*|Connection recv data is binary.|readOnly|
|isFinal|*BOOL*|Final data for current message was recieved.|readOnly|
|msgQLen|*INT*|Number of messages in input queue.|readOnly|
|recvCnt|*INT*|Number of recieves.|readOnly|
|recvLast|*TIME_T*|Time of last recv.|readOnly|
|redirLast|*TIME_T*|Time of last redirect.|readOnly|
|redirCnt|*INT*|Count of redirects.|readOnly|
|sentCnt|*INT*|Number of sends.|readOnly|
|sentLast|*TIME_T*|Time of last send.|readOnly|
|sentErrCnt|*INT*|Number of sends.|readOnly|
|sentErrLast|*TIME_T*|Time of last sendErr.|readOnly|
|sentErrLast|*TIME_T*|Time of last sendErr.|readOnly|
|uploadCnt|*INT*|Number of uploads.|readOnly|
|uploadEnd|*TIME_T*|Time of upload end.|readOnly|
|uploadLast|*TIME_T*|Time of last upload input.|readOnly|
|uploadStart|*TIME_T*|Time of upload start.|readOnly|


## Zvfs



Commands for mounting and accessing .zip files as a filesystem.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|append|(archive:string, filelist:array, path:string&#124;null=void, filelist2:array=void, path2:string&#124;null=void, ...):void |Like 'create()', but appends to an existing archive (with no dup checking).|
|create|(archive:string, filelist:array, path:string&#124;null=void, filelist2:array=void, path2:string&#124;null=void, ...):void |Create a zip with the given files in prefix path. This command creates a zip archive and adds files to it. Files are relative the given 'path', or the current directory. If the destignation file already exist but is not an archive (eg. an executable), zip data is appended to the end of the file. If the existing file is already an archive, an error will be thrown. To truncate an existing archive, use zvfs.truncate(). Or use zvfs.append() instead.     zvfs.create('foo.zip',['main.js', 'bar.js'], 'src', ['a.html', 'css/a.css'], 'html');|
|deflate|(data:string):string |Compress string using zlib deflate.|
|inflate|(data:string):string |Uncompress string using zlib inflate.|
|list|(archive:string):array |List files in archive. Return contents of zip directory as an array of arrays. The first element contains the labels, ie:  [ 'Name', 'Special', 'Offset', 'Bytes', 'BytesCompressed' ] |
|mount|(archive:string, mountdir:string=void):string |Mount zip on mount point. Read a ZIP archive and make entries in the virutal file hash table for all files contained therein.|
|names|(mountdir:string=void):array |Return all zvfs mounted zips, or archive for specified mount. Given an mount point argument, returns the archive for it. Otherwise, returns an array of mount points|
|offset|(archive:string):number |Return the start offset of zip data. Opens and scans the file to determine start of zip data and truncate this off the end of the file.  For ordinary zip archives, the resulting truncated file will be of zero length. If an optional bool argument can disable errors. In any case, the start offset of zip data (or 0) is returned.|
|stat|(filename:string):object |Return details on file in zvfs mount. Return details about the given file in the ZVFS.  The information consists of (1) the name of the ZIP archive that contains the file, (2) the size of the file after decompressions, (3) the compressed size of the file, and (4) the offset of the compressed data in the archive.|
|truncate|(archive:string, noerror:boolean=false):number |Truncate zip data from archive. Opens and scans the file to determine start of zip data and truncate this off the end of the file.  For ordinary zip archives, the resulting truncated file will be of zero length. If an optional bool argument can disable errors. In any case, the start offset of zip data (or 0) is returned.|
|unmount|(archive:string):void |Unmount zip.|


## console



Console input and output to stderr.


### Methods
|Method|Function Argument Types|Description|
|---|---|---|
|assert|([options](#options-consoleassert):object=void):void |Same as System.assert().|
|error|(val, ...):void |Same as log but adding prefix ERROR:.|
|input|(prompt:null&#124;string=''):string&#124;void |Read input from the console: if prompt uses linenoise line editing.|
|log|(val, ...):void |Like System.puts, but goes to stderr and includes file:line..|
|logp|(val, ...):void |Same as console.log, but first arg is string prefix and if second is a boolean it controls output.|
|printf|(format:string, ...):void |Same as System.printf but goes to stderr.|
|puts|(val, ...):void |Same as System.puts, but goes to stderr.|
|warn|(val, ...):void |Same as log.|
### Options "console.assert"
|Option|Type|Description|Flags|
|---|---|---|---|
|mode|*STRKEY*|Action when assertion fails. Default from Interp.assertMode. (one of: **throw**, **log**, **puts**)||
|noStderr|*BOOL*|Logged msg to stdout. Default from Interp.noStderr.||
