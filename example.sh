#!/bin/bash

# 保存以下脚本为 example.sh

#!/bin/bash

echo '.mode columns' | sqlite3 database.db
echo '.headers on' | sqlite3 database.db
sqlite3 database.db ".database"

