#!/bin/bash

newUser='testuser'
newDbPassword='testpwd'
newDb='bumondb'
host=localhost
#host='%'
 
commands="CREATE DATABASE \`${newDb}\`;CREATE USER '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT USAGE ON *.* TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT ALL privileges ON \`${newDb}\`.*
TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';FLUSH PRIVILEGES;"

mysql -u root --password="" -e "${commands}"
cat bumon-test.sql | mysql -u ${newUser} -p${newDbPassword} ${newDb}
