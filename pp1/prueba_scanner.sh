programas_frag=($(ls samples | grep -oP '.*\.frag' | grep -oP '.*\.'))
programas_decaf=($(ls samples | grep -oP '.*\.decaf' | grep -oP '.*\.'))

mkdir difs
mkdir out_mbc

for i in "${programas_frag[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"frag") > out_mbc/$i"out" 2>&1 # imprime std output y std error
   (diff --text out_mbc/$i"out" "samples/"$i"out") > difs/$i"diff"
   # rm temp_file_out.txt
done


for i in "${programas_decaf[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"decaf") > out_mbc/$i"out" 2>&1
   (diff --text out_mbc/$i"out" "samples/"$i"out") > difs/$i"diff"
   # (diff --text temp_file_out.txt "samples/"$i"out") > difs/$i"diff"
   # rm temp_file_out.txt
done