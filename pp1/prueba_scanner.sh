programas_frag=($(ls samples | grep -oP '.*\.frag' | grep -oP '.*\.'))
programas_decaf=($(ls samples | grep -oP '.*\.decaf' | grep -oP '.*\.'))

mkdir difs

for i in "${programas_frag[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"frag") > temp_file_out.txt
   (diff temp_file_out.txt "samples/"$i"out") > difs/diff_out_$i.txt
   rm temp_file_out.txt
done


for i in "${programas_decaf[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"decaf") > temp_file_out.txt
   (diff temp_file_out.txt "samples/"$i"out") > difs/diff_out_$i.txt
   rm temp_file_out.txt
done