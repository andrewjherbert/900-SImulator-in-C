#!bin/sh
set +x
rm -rf .reader .punch

./to900text ./bin/misc/cntdwn.txt
echo here
cp .reader cntdwn_cards
./emu900 -j=8181 -reader=./bin/misc/cntdwn -ttyin=cntdwn_cards
rm cntdwn_cards
