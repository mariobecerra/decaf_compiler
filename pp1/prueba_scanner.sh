programas_frag=($(ls samples | grep -oP '.*\.frag' | grep -oP '.*\.'))
programas_decaf=($(ls samples | grep -oP '.*\.decaf' | grep -oP '.*\.'))

mkdir difs
mkdir out_mbc

for i in "${programas_frag[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"frag") > out_mbc/$i"out1" 2>&1 # imprime std output y std error
   tr < out_mbc/$i"out1" -d '\000' > out_mbc/$i"out" # Quita caracteres nulos
   (diff --text out_mbc/$i"out" "samples/"$i"out") > difs/$i"diff"
   # rm temp_file_out.txt
   rm out_mbc/$i"out1"
done


for i in "${programas_decaf[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"decaf") > out_mbc/$i"out1" 2>&1
   tr < out_mbc/$i"out1" -d '\000' > out_mbc/$i"out"
   (diff --text out_mbc/$i"out" "samples/"$i"out") > difs/$i"diff"
   # (diff --text temp_file_out.txt "samples/"$i"out") > difs/$i"diff"
   # rm temp_file_out.txt
   rm out_mbc/$i"out1"
done