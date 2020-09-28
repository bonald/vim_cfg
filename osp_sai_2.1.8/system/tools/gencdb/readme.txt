Welcome to gencdb
------------------

Gencdb is a series of tools to generate CDB source codes from schema files in directory 'define'.
These are 5 tools start with prefix 'gen_':
1. gen_node.py
   generate CDB node
2. gen_define.py
   generate CDB TBL/DS define and api header files
3. gen_api.py
   generate CDB TBL/DS api c files
4. gen_store.py
   generate CDB TBL/DS store c files
5. gen_check.py
   check generate lib and proto/lib
6. genall.sh
   call item 1-5
7. genlog.py

Input files
------------------
in directory 'define', 'user_define'

Output files
------------------
in directory 'lib'
