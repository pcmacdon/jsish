-- Schema file for Ledgerjs database.
PRAGMA foreign_keys = ON;
CREATE TABLE IF NOT EXISTS atype (
    atype INTEGER PRIMARY KEY, 
    atypename UNIQUE NOT NULL 
    );
    
insert or replace into atype values(1,'expense');
insert or replace into atype values(2,'revenue');
insert or replace into atype values(3,'asset');
insert or replace into atype values(4,'liability');
insert or replace into atype values(5,'equity'); 

CREATE TABLE IF NOT EXISTS accts (
    aid INTEGER PRIMARY KEY,
    aname TEXT UNIQUE NOT NULL,
    aobal REAL DEFAULT 0.0,
    attl REAL DEFAULT 0.0,
    acbal REAL DEFAULT 0.0,
    arbal REAL DEFAULT 0.0,    
    abudget INT DEFAULT 0  CHECK(abudget==0 OR abudget==1),
    atransnums INT DEFAULT 0,
    adefaultnum DEFAULT '',
    anum INT UNIQUE DEFAULT NULL,
    ainstname DEFAULT '',
    ainstaddr1 DEFAULT '',
    ainstaddr2 DEFAULT '',
    ainstcity DEFAULT '', 
    ainstzip DEFAULT '',
    ainstphone DEFAULT '',
    ainstfax DEFAULT '',
    ainstemail DEFAULT '',
    ainstcontact DEFAULT '',
    ainstnotes DEFAULT '',
    acatagory INT NOT NULL DEFAULT 0 CHECK(acatagory==0 OR acatagory==1),
    ataxed INT NOT NULL DEFAULT 0 CHECK(ataxed==0 OR ataxed==1),
    atype DEFAULT 1, 
    apid INT DEFAULT 0,
    arecebals NUMERIC DEFAULT 0,
    aoptions TEXT DEFAULT '',
    FOREIGN KEY(atype)    REFERENCES atype(atype) 
    );

CREATE TABLE IF NOT EXISTS schedule (
    tsched INTEGER PRIMARY KEY, 
    sedate DATE DEFAULT NULL, 
    speriod INT DEFAULT NULL, 
    speriodnum INT DEFAULT NULL, 
    smonthday DEFAULT NULL, 
    smonthdaynum DEFAULT NULL, 
    slast DEFAULT NULL 
    );

CREATE TABLE IF NOT EXISTS reconcile (
    treco INTEGER PRIMARY KEY, 
    aid INTEGER not NULL, 
    rdate DATE NOT NULL DEFAULT CURRENT_DATE, 
    rmemo TEXT DEFAULT NULL, 
    FOREIGN KEY(aid)    REFERENCES accts(aid) 
    );

CREATE TABLE IF NOT EXISTS tgroup (
    tgroup INTEGER PRIMARY KEY, 
    tlist TEXT DEFAULT '', 
    alist TEXT DEFAULT '', 
    cnt INT DEFAULT 0, 
    tnum DEFAULT '', 
    tpayee TEXT DEFAULT '', 
    tmemo TEXT DEFAULT '', 
    tdate DATE DEFAULT CURRENT_DATE, 
    tsched INTEGER DEFAULT NULL, 
    tflags INTEGER DEFAULT 0, 
    FOREIGN KEY(tsched)  REFERENCES schedule(tsched) 
    );
  
CREATE TABLE IF NOT EXISTS trans (
    tid INTEGER PRIMARY KEY, 
    tsum REAL DEFAULT 0, 
    -- truntot REAL DEFAULT 0, 
    tgroup INTEGER NOT NULL, 
    aid INTEGER NOT NULL, 
    treco INTEGER DEFAULT NULL, 
    FOREIGN KEY(tgroup) REFERENCES tgroup(tgroup), 
    FOREIGN KEY(aid)    REFERENCES accts(aid), 
    FOREIGN KEY(treco)  REFERENCES reconcile(treco) 
    );
    
CREATE TABLE IF NOT EXISTS truntot (
    tid INTEGER PRIMARY KEY,
    truntot REAL DEFAULT 0,
    FOREIGN KEY(tid) REFERENCES trans(tid)
    );
  
CREATE TABLE IF NOT EXISTS options (
    name TEXT PRIMARY KEY, 
    value TEXT DEFAULT '' 
    );

CREATE INDEX IF NOT EXISTS transaid on trans(aid);
CREATE INDEX IF NOT EXISTS transtgroup on trans(tgroup);
CREATE INDEX IF NOT EXISTS transtdate on tgroup(tdate);

CREATE TABLE IF NOT EXISTS undolog (
    seq integer primary key,
    datestamp DATE DEFAULT current_timestamp, 
    sql TEXT DEFAULT ''
    );
    
CREATE TABLE IF NOT EXISTS history (
    seq integer primary key,
    datestamp DATE DEFAULT current_timestamp, 
    typ TEXT NOT NULL CHECK(typ=='I' OR typ=='U' OR typ=='D'),
    json TEXT NOT NULL
    );
    
CREATE TABLE IF NOT EXISTS master (
    seq integer primary key,
    datestamp DATE DEFAULT current_timestamp, 
    dbname TEXT NOT NULL,
    password TEXT NOT NULL
    );
 
