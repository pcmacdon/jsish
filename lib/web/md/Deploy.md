Deploy
======
[Back to Index](Index.md "Goto Jsi Documentation Index")

Jsi code can be deployed as an archive (<b>zip, sqlar</b>) or <b>fossil</b> repo which:

- contains one or several applications.
- provides all support files required by application: **.jsi**, **.html**, etc.
- supports remote update, syncing and multiple versions *(for fossil)*.


Typically a *deploy* contains the file "main.jsi", plus directories
of packages which can be run, as in:

``` bash
jsish -a jsi-app.zip Ledger
```

## Fossil

Fossil archives offer the ability for safe software update:

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

## Publish

During development of an App-Archive, there is the additional step
of application changes getting pushed up to the repos, with appropriate
tags to enforce dependencies.


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

## Archive

Jsi supports mounting archives, to serve out content:

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

## Security

Jsi code can **NOT** be executed remotely as in:

    jsish http://host.com/foo.jsi;  # Does not work.

This would be a huge security hole.
Although it would be simple enough to implement.
