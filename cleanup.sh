#/bin/bash
#

pattern="._*"
echo "Deleting files $pattern"
for file in $(sudo find . -name "$pattern"); do
    echo $file
    rm $file
done 