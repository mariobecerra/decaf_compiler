programas_frag=($(ls samples | grep -oP '.*\.frag' | grep -oP '.*\.'))
programas_decaf=($(ls samples | grep -oP '.*\.decaf' | grep -oP '.*\.'))

output_folder=output_dcc
difs_folder=difs

mkdir $difs_folder
mkdir $output_folder

for i in "${programas_frag[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"frag") > $output_folder/$i"out1" 2>&1 # imprime std output y std error
   tr < $output_folder/$i"out1" -d '\000' > $output_folder/$i"out" # Quita caracteres nulos
   (diff --text out_mbc/$i"out" "samples/"$i"out") > $difs_folder/$i"diff"
   # rm temp_file_out.txt
   rm $output_folder/$i"out1"
done


for i in "${programas_decaf[@]}"
do
   : 
   # do whatever on $i
   (./dcc < "samples/"$i"decaf") > $output_folder/$i"out1" 2>&1
   tr < $output_folder/$i"out1" -d '\000' > $output_folder/$i"out"
   (diff --text $output_folder/$i"out" "samples/"$i"out") > $difs_folder/$i"diff"
   # (diff --text temp_file_out.txt "samples/"$i"out") > $difs_folder/$i"diff"
   # rm temp_file_out.txt
   rm $output_folder/$i"out1"
done
