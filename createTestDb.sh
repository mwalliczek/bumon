#! /bin/bash

newUser='testuser'
newDbPassword='testpwd'
newDb='bumondb'
host=localhost
#host='%'
 
commands="CREATE DATABASE IF NOT EXISTS \`${newDb}\`;CREATE USER IF NOT EXISTS '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT USAGE ON *.* TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';GRANT ALL privileges ON \`${newDb}\`.*
TO '${newUser}'@'${host}' IDENTIFIED BY '${newDbPassword}';FLUSH PRIVILEGES;"

echo "${commands}" | /usr/bin/mysql -u root -p
cat bumon-test.sql | /usr/bin/mysql -u ${newUser} -p${newDbPassword} ${newDb}
