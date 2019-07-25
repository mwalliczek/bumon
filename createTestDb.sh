#!/bin/bash

newUser='testuser'
newDbPassword='testpwd'
newDb='bumondb'
host=localhost
#host='%'
 
commands="CREATE DATABASE IF NOT EXISTS \`${newDb}\`;CREATE USER IF NOT EXISTS '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT USAGE ON *.* TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT ALL privileges ON \`${newDb}\`.*
TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';FLUSH PRIVILEGES;"

echo ${commands}
mysql -u root -p -e "${commands}"
cat bumon-test.sql | mysql -u ${newUser} -p${newDbPassword} ${newDb}
