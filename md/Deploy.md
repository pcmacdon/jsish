Deploy
======
<div id="sectmenu"></div>
Jsi code can be deployed as an archive (<b>zip, sqlar</b>) or <b>fossil</b> repo which:

- contains one or several applications.
- provides all support files required by application: **.jsi**, **.html**, etc.
- supports remote update, syncing and multiple versions *(for fossil)*.


Typically a *deploy* contains the file "main.jsi", plus directories
of [packages](Coding.md#files) which can be run, as in:

    jsish -a jsi-app.zip Ledger

Fossil
------
Fossil archives offer the ability for safe software update:

<!--
        *********************************
        *              .--------------. *
        *              |   Internet   | *
        * .---------.  |   .-----.    | *
        * | Browser |  |  | Fossil|   | *
        * '----+----'  |   '--+--'    | *
        *      ^       |      |       | *
        *      |       '------+-------' *
        *      |              |         *
        *      v              v         *
        *  .-------.     .---------.    *
        * | Jsish+  |    | App     |    *
        * | Zvfs    |<---+ Fossil  |    *
        *  '-------'     '---------'    *
        ********************************* -->
<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="224" width="256" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 8,32 L 8,64 " style="fill:none;"/>
<path d="M 8,160 L 8,176 " style="fill:none;"/>
<path d="M 48,72 L 48,136 " style="fill:none;"/>
<path d="M 88,32 L 88,64 " style="fill:none;"/>
<path d="M 88,160 L 88,176 " style="fill:none;"/>
<path d="M 112,0 L 112,96 " style="fill:none;"/>
<path d="M 128,144 L 128,192 " style="fill:none;"/>
<path d="M 168,64 L 168,136 " style="fill:none;"/>
<path d="M 208,144 L 208,192 " style="fill:none;"/>
<path d="M 232,0 L 232,96 " style="fill:none;"/>
<path d="M 112,0 L 232,0 " style="fill:none;"/>
<path d="M 8,32 L 88,32 " style="fill:none;"/>
<path d="M 152,32 L 184,32 " style="fill:none;"/>
<path d="M 8,64 L 88,64 " style="fill:none;"/>
<path d="M 152,64 L 184,64 " style="fill:none;"/>
<path d="M 112,96 L 232,96 " style="fill:none;"/>
<path d="M 24,144 L 72,144 " style="fill:none;"/>
<path d="M 128,144 L 208,144 " style="fill:none;"/>
<path d="M 96,176 L 128,176 " style="fill:none;"/>
<path d="M 24,192 L 72,192 " style="fill:none;"/>
<path d="M 128,192 L 208,192 " style="fill:none;"/>
<path d="M 152,32 C 135.2,32 136,48 136,48 " style="fill:none;"/>
<path d="M 184,32 C 200.8,32 200,48 200,48 " style="fill:none;"/>
<path d="M 152,64 C 135.2,64 136,48 136,48 " style="fill:none;"/>
<path d="M 184,64 C 200.8,64 200,48 200,48 " style="fill:none;"/>
<path d="M 24,144 C 7.199999999999999,144 8,160 8,160 " style="fill:none;"/>
<path d="M 72,144 C 88.8,144 88,160 88,160 " style="fill:none;"/>
<path d="M 24,192 C 7.199999999999999,192 8,176 8,176 " style="fill:none;"/>
<path d="M 72,192 C 88.8,192 88,176 88,176 " style="fill:none;"/>
<polygon points="176,136 164,130.4 164,141.6 "  style="stroke:none" transform="rotate(90,168,136 )"/>
<polygon points="104,176 92,170.4 92,181.6 "  style="stroke:none" transform="rotate(180,96,176 )"/>
<polygon points="56,136 44,130.4 44,141.6 "  style="stroke:none" transform="rotate(90,48,136 )"/>
<polygon points="56,72 44,66.4 44,77.6 "  style="stroke:none" transform="rotate(270,48,72 )"/>
<g transform="translate(0,0)"><text text-anchor="middle" x="144" y="20">I</text><text text-anchor="middle" x="152" y="20">n</text><text text-anchor="middle" x="160" y="20">t</text><text text-anchor="middle" x="168" y="20">e</text><text text-anchor="middle" x="176" y="20">r</text><text text-anchor="middle" x="184" y="20">n</text><text text-anchor="middle" x="192" y="20">e</text><text text-anchor="middle" x="200" y="20">t</text><text text-anchor="middle" x="24" y="52">B</text><text text-anchor="middle" x="32" y="52">r</text><text text-anchor="middle" x="40" y="52">o</text><text text-anchor="middle" x="48" y="52">w</text><text text-anchor="middle" x="56" y="52">s</text><text text-anchor="middle" x="64" y="52">e</text><text text-anchor="middle" x="72" y="52">r</text><text text-anchor="middle" x="152" y="52">F</text><text text-anchor="middle" x="160" y="52">o</text><text text-anchor="middle" x="168" y="52">s</text><text text-anchor="middle" x="176" y="52">s</text><text text-anchor="middle" x="184" y="52">i</text><text text-anchor="middle" x="192" y="52">l</text><text text-anchor="middle" x="24" y="164">J</text><text text-anchor="middle" x="32" y="164">s</text><text text-anchor="middle" x="40" y="164">i</text><text text-anchor="middle" x="48" y="164">s</text><text text-anchor="middle" x="56" y="164">h</text><text text-anchor="middle" x="64" y="164">+</text><text text-anchor="middle" x="144" y="164">A</text><text text-anchor="middle" x="152" y="164">p</text><text text-anchor="middle" x="160" y="164">p</text><text text-anchor="middle" x="24" y="180">Z</text><text text-anchor="middle" x="32" y="180">v</text><text text-anchor="middle" x="40" y="180">f</text><text text-anchor="middle" x="48" y="180">s</text><text text-anchor="middle" x="144" y="180">F</text><text text-anchor="middle" x="152" y="180">o</text><text text-anchor="middle" x="160" y="180">s</text><text text-anchor="middle" x="168" y="180">s</text><text text-anchor="middle" x="176" y="180">i</text><text text-anchor="middle" x="184" y="180">l</text></g></g></svg>


Now when an application starts it can issue a fossil pull.
This means that potentially all versions of software are available
at program start-up.

But if a script update occurs that requires a newer version of Jsish, this could be dangerous.
However we can avoid this problem by having the Fossil archive tag releases not only
with version numbers, but also with Jsish prerequisites.  Thus the archive
manager can mount the latest compatible version.

This is what is meant by safe update.  Although all releases (past and present) may be available,
the latest safe one will be chosen for us automatically.
Or we can choose to fallback previous versions without re-installing.
Thus like /zvfs, dependency issues are avoided.

Publish
----
During development of an App-Archive, there is the additional step
of application changes getting pushed up to the repos, with appropriate
tags to enforce dependencies.

<!--
        *********************************
        *              .--------------. *
        *              |   Internet   | *
        * .---------.  |   .-----.    | *
        * | Browser |  |  | Fossil|   | *
        * '----+----'  |   '--+--'    | *
        *      ^       |      ^       | *
        *      |       '------+-------' *
        *      |              |         *
        *      v              v         *
        *  .-------.     .---------.    *
        * | Jsish+  |    | App     |    *
        * | Zvfs    |<-- >+ Fossil  |    *
        *  '-------'     '---------'    *
        ********************************* -->
<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="224" width="256" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 8,32 L 8,64 " style="fill:none;"/>
<path d="M 8,160 L 8,176 " style="fill:none;"/>
<path d="M 48,72 L 48,136 " style="fill:none;"/>
<path d="M 88,32 L 88,64 " style="fill:none;"/>
<path d="M 88,160 L 88,176 " style="fill:none;"/>
<path d="M 112,0 L 112,96 " style="fill:none;"/>
<path d="M 128,144 L 128,192 " style="fill:none;"/>
<path d="M 168,72 L 168,136 " style="fill:none;"/>
<path d="M 208,144 L 208,192 " style="fill:none;"/>
<path d="M 232,0 L 232,96 " style="fill:none;"/>
<path d="M 112,0 L 232,0 " style="fill:none;"/>
<path d="M 8,32 L 88,32 " style="fill:none;"/>
<path d="M 152,32 L 184,32 " style="fill:none;"/>
<path d="M 8,64 L 88,64 " style="fill:none;"/>
<path d="M 152,64 L 184,64 " style="fill:none;"/>
<path d="M 112,96 L 232,96 " style="fill:none;"/>
<path d="M 24,144 L 72,144 " style="fill:none;"/>
<path d="M 128,144 L 208,144 " style="fill:none;"/>
<path d="M 96,176 L 120,176 " style="fill:none;"/>
<path d="M 24,192 L 72,192 " style="fill:none;"/>
<path d="M 128,192 L 208,192 " style="fill:none;"/>
<path d="M 152,32 C 135.2,32 136,48 136,48 " style="fill:none;"/>
<path d="M 184,32 C 200.8,32 200,48 200,48 " style="fill:none;"/>
<path d="M 152,64 C 135.2,64 136,48 136,48 " style="fill:none;"/>
<path d="M 184,64 C 200.8,64 200,48 200,48 " style="fill:none;"/>
<path d="M 24,144 C 7.199999999999999,144 8,160 8,160 " style="fill:none;"/>
<path d="M 72,144 C 88.8,144 88,160 88,160 " style="fill:none;"/>
<path d="M 24,192 C 7.199999999999999,192 8,176 8,176 " style="fill:none;"/>
<path d="M 72,192 C 88.8,192 88,176 88,176 " style="fill:none;"/>
<polygon points="176,136 164,130.4 164,141.6 "  style="stroke:none" transform="rotate(90,168,136 )"/>
<polygon points="176,72 164,66.4 164,77.6 "  style="stroke:none" transform="rotate(270,168,72 )"/>
<polygon points="128,176 116,170.4 116,181.6 "  style="stroke:none" transform="rotate(0,120,176 )"/>
<polygon points="104,176 92,170.4 92,181.6 "  style="stroke:none" transform="rotate(180,96,176 )"/>
<polygon points="56,136 44,130.4 44,141.6 "  style="stroke:none" transform="rotate(90,48,136 )"/>
<polygon points="56,72 44,66.4 44,77.6 "  style="stroke:none" transform="rotate(270,48,72 )"/>
<g transform="translate(0,0)"><text text-anchor="middle" x="144" y="20">I</text><text text-anchor="middle" x="152" y="20">n</text><text text-anchor="middle" x="160" y="20">t</text><text text-anchor="middle" x="168" y="20">e</text><text text-anchor="middle" x="176" y="20">r</text><text text-anchor="middle" x="184" y="20">n</text><text text-anchor="middle" x="192" y="20">e</text><text text-anchor="middle" x="200" y="20">t</text><text text-anchor="middle" x="24" y="52">B</text><text text-anchor="middle" x="32" y="52">r</text><text text-anchor="middle" x="40" y="52">o</text><text text-anchor="middle" x="48" y="52">w</text><text text-anchor="middle" x="56" y="52">s</text><text text-anchor="middle" x="64" y="52">e</text><text text-anchor="middle" x="72" y="52">r</text><text text-anchor="middle" x="152" y="52">F</text><text text-anchor="middle" x="160" y="52">o</text><text text-anchor="middle" x="168" y="52">s</text><text text-anchor="middle" x="176" y="52">s</text><text text-anchor="middle" x="184" y="52">i</text><text text-anchor="middle" x="192" y="52">l</text><text text-anchor="middle" x="24" y="164">J</text><text text-anchor="middle" x="32" y="164">s</text><text text-anchor="middle" x="40" y="164">i</text><text text-anchor="middle" x="48" y="164">s</text><text text-anchor="middle" x="56" y="164">h</text><text text-anchor="middle" x="64" y="164">+</text><text text-anchor="middle" x="144" y="164">A</text><text text-anchor="middle" x="152" y="164">p</text><text text-anchor="middle" x="160" y="164">p</text><text text-anchor="middle" x="24" y="180">Z</text><text text-anchor="middle" x="32" y="180">v</text><text text-anchor="middle" x="40" y="180">f</text><text text-anchor="middle" x="48" y="180">s</text><text text-anchor="middle" x="144" y="180">F</text><text text-anchor="middle" x="152" y="180">o</text><text text-anchor="middle" x="160" y="180">s</text><text text-anchor="middle" x="168" y="180">s</text><text text-anchor="middle" x="176" y="180">i</text><text text-anchor="middle" x="184" y="180">l</text></g></g></svg>


The publish script pulls the dependencies automatically from the
app files and adds the requisite tags.

A fossil-deploy is of particular interest as it can:

- run client apps that keep in sync with a master.
- host development that remotely syncs up the master.
- serve out Internet web applications from a master.

Most importantly, deploys automatically use the latest available
version at startup.

**Note**:
    For an example see the [jsi-app](https://jsish.org/jsi-app) deploy which
    hosts the [online Ledger demo](https://jsish.org/App10/Ledger/html/main.htmli).

Jsish can run applications directly from downloaded fossil repositories, eg:

    fossil clone http://jsish.org/jsi-app jsi-app.fossil
    jsish -a jsi-app.fossil Ledger -help

### Updating
To bring a repository up-to-date use:

    fossil pull -R jsi-app.fossil

Or run with the update option:

    jsish -a -update true jsi-app.fossil Ledger

### Versioning
Jsish makes use of special tags in the repository to determine which checkin to mount:

  *  ver-M.N: a release version tag.
  *  prereq-M.N: version of Jsish required for a ver-M.N tag.

A "prereq" tag may be associated only to commits with a "ver" tag.

**Note**:
    Version tags use the float format, rather than "XX.YY.ZZ".

  *  ver-1.02 ==> "1.2"
  *  ver-1.0203 ==> "1.2.3"

The rules for determining the default version to mount are:

  *  No **prereq** or **ver** tags, the **trunk** is mounted.
  *  No **prereq** tags higher than Jsish version, the highest **ver** tag is mounted.
  *  Else, the highest **ver** found where **prereq** is not > Jsish version.

This means it is generally safe to keep repositories up-to-date
without worrying about Jsish dependancies.

Explicit versions can be mounted using:

    jsish -a -version ver-1.0 jsi-app.fossil Ledger
    jsish -a -version 2018-09-07 jsi-app.fossil Ledger

For a list of available tags use:

    fossil tag list -R jsi-app.fossil

Using fossil provides many more benefits.  Such as easy checking out of source,
apply fixes, and generating dif files to submit bug fixes.
More on this to come.

Archive
-------
Jsi supports mounting archives, to serve out content:

<!--
        *****************************
        * .---------.               *
        * | Browser |               *
        * '----+----'               *
        *      ^                    *
        *      |                    *
        *      v                    *
        *  .-------.    .----+----. *
        * | Jsish+  |   | App     | *
        * | Zvfs    |<--+ Archive | *
        *  '-------'    '---------' *
        ***************************** -->

<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="176" width="224" style="margin:0 auto 0 auto;"><g transform="translate(8,16 )">
<path d="M 8,0 L 8,32 " style="fill:none;"/>
<path d="M 8,112 L 8,128 " style="fill:none;"/>
<path d="M 48,40 L 48,88 " style="fill:none;"/>
<path d="M 88,0 L 88,32 " style="fill:none;"/>
<path d="M 88,112 L 88,128 " style="fill:none;"/>
<path d="M 120,96 L 120,144 " style="fill:none;"/>
<path d="M 200,96 L 200,144 " style="fill:none;"/>
<path d="M 8,0 L 88,0 " style="fill:none;"/>
<path d="M 8,32 L 88,32 " style="fill:none;"/>
<path d="M 24,96 L 72,96 " style="fill:none;"/>
<path d="M 120,96 L 200,96 " style="fill:none;"/>
<path d="M 96,128 L 120,128 " style="fill:none;"/>
<path d="M 24,144 L 72,144 " style="fill:none;"/>
<path d="M 120,144 L 200,144 " style="fill:none;"/>
<path d="M 24,96 C 7.199999999999999,96 8,112 8,112 " style="fill:none;"/>
<path d="M 72,96 C 88.8,96 88,112 88,112 " style="fill:none;"/>
<path d="M 24,144 C 7.199999999999999,144 8,128 8,128 " style="fill:none;"/>
<path d="M 72,144 C 88.8,144 88,128 88,128 " style="fill:none;"/>
<polygon points="104,128 92,122.4 92,133.6 "  style="stroke:none" transform="rotate(180,96,128 )"/>
<polygon points="56,88 44,82.4 44,93.6 "  style="stroke:none" transform="rotate(90,48,88 )"/>
<polygon points="56,40 44,34.4 44,45.6 "  style="stroke:none" transform="rotate(270,48,40 )"/>
<g transform="translate(0,0)"><text text-anchor="middle" x="24" y="20">B</text><text text-anchor="middle" x="32" y="20">r</text><text text-anchor="middle" x="40" y="20">o</text><text text-anchor="middle" x="48" y="20">w</text><text text-anchor="middle" x="56" y="20">s</text><text text-anchor="middle" x="64" y="20">e</text><text text-anchor="middle" x="72" y="20">r</text><text text-anchor="middle" x="24" y="116">J</text><text text-anchor="middle" x="32" y="116">s</text><text text-anchor="middle" x="40" y="116">i</text><text text-anchor="middle" x="48" y="116">s</text><text text-anchor="middle" x="56" y="116">h</text><text text-anchor="middle" x="64" y="116">+</text><text text-anchor="middle" x="136" y="116">A</text><text text-anchor="middle" x="144" y="116">p</text><text text-anchor="middle" x="152" y="116">p</text><text text-anchor="middle" x="24" y="132">Z</text><text text-anchor="middle" x="32" y="132">v</text><text text-anchor="middle" x="40" y="132">f</text><text text-anchor="middle" x="48" y="132">s</text><text text-anchor="middle" x="136" y="132">A</text><text text-anchor="middle" x="144" y="132">r</text><text text-anchor="middle" x="152" y="132">c</text><text text-anchor="middle" x="160" y="132">h</text><text text-anchor="middle" x="168" y="132">i</text><text text-anchor="middle" x="176" y="132">v</text><text text-anchor="middle" x="184" y="132">e</text></g></g></svg>

        
Archives are of interest because they can quickly be pulled from a fossil repository.

### Zip
Zip archive support is the simplest and oldest supported archive.  It features:

  *  Native support (ie. mounts without "-a").
  *  Mounts are visible within sub-interps.
  *  Zip format is widely used.

To pull down the head revision of **jsi-app** use:


    wget -O jsi-app.zip http://jsish.org/jsi-app/zip

or jsish can be used:


    jsish --wget -O jsi-app.zip http://jsish.org/jsi-app/zip

Alternatively the [fossil UI](http://jsish.org/jsi-app) supports
"Zip Archive" download.

**Note**:
    One downside to the zip format is it's readonly as it does not support in-place updates.


### Sqlar
The [sqlar](https://www.sqlite.org/sqlar/doc/trunk/README) stores compressed files in a small
sqlite database.  Although it is not widely used it is more easily supports updating in-place.

You can pull down the head reversion of "jsi-app" with:

    wget -O jsi-app.zip http://jsish.org/jsi-app/zip

The [fossil API](https://jsish.org/jsi-app) provides
"Sql Archive" download.

Security
----
Jsi code can **NOT** be executed remotely as in:

    jsish http://host.com/foo.jsi;  # Does not work.

This would be a huge security hole.
Although it would be simple enough to implement.
