A simple proposal for a persistance model.

Using @zachflower [PR9](https://github.com/mudcoders/guildmud/pull/9) as a guideline, let's implement data persistance in sqlite.

We have three options to persist the data in sqlite, from simpler to most complex:

1. Give an id to every data structure in the system, and store them in a single table. Content of the data structure is the persisted data structure (persistance model to be defined)

![DB Model](file:///Users/rogersm/Dropbox/dev/guildmud/doc/db_model1.png)

2. Develop the previous model and add an additional parent/child table to track which objects are contained inside other objects.

![DB Model](file:///Users/rogersm/Dropbox/dev/guildmud/doc/db_model2.png)

3. Do a full persistance where each data structure to be persisted is mapped to a field/table in the database.

Each method has some pro/cons, summarized in this table:

|      | Single Table (#1)  |  Single Table + parent/child (#2) | Full Persistance (#3) |
| ---- | :----------------- | :-------------------------------- | :-------------------- |
| PROs | Easiest to implement. <br/> Minimum code/sql coupling needed. <br/> Adding functionality does not imply database model changes. | Easy to implement.  Allows to track parent/child relationships. Adding functionality does not imply database model changes. | Allows full use of data modeling tools.<br/>                                    |
| CONs | Using sqlite only as a data storage instead of disk files.                                                                      | Not so easy to implement as a single table model.                                                                           | Difficult to add functionality: highly coupled C and SQL model code means any code change implies SQL change. |
For the objective of this simple mud, we will use #2.

 