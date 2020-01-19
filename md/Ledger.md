Ledger
======
<div id="sectmenu"></div>
[Jsi](https://jsish.org/jsi) Ledger is a simple Web-UI based accounting program.

Account and transaction data is stored in an Sqlite database, which
is backed up as an exported checked into a fossil repository.

The database can be inspected via menu **Help/Database**.

Keyboard Shortcuts
------------------

- Alt-n : Create a new transaction
- Alt-a : Move ahead to next page of transactions
- Alt-b : Move back to previous page of transactions


Sorting
-------

To sort by columns, click on the column header.  Clicking again will
reverse the sort direction.

Search
------

An input shows the row offset in transactions.  This can be edited to
skip a specific number of rows.

The input can be prefixed (before the colon) by a pattern.
If pattern contains a "*", it uses case-sensitive **GLOB** matching.
Otherwise **LIKE** matching will be used, and if there is no "%" already,
it weill be added to the begining and end.

Reports
-------

Reports available under **Admin** include:

- Account Summary
- Trial Balance
- General Ledger
- Totals by Payee
- Reconcilation

