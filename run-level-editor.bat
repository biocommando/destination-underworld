cd  dataloss
echo "<body onload='open(`http://localhost:3000/editor/du-edit.html`, `_self`)'>" > open-du-edit.html
open-du-edit.html
call node editor/du-edit-server.js