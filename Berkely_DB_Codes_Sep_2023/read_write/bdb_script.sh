exampleName="read_write"

studentID="1234"
course="BDB"
user=`whoami`
dateTime=`date`
dateTime2=`date "+%d.%m.%Y-%H.%M.%S"`

outputFile=$user"_"$course"_"$exampleName"_"$dateTime2".txt"
line="----------------------------------------------------------------------------"

echo $line | tee $outputFile
echo "Student ID: $studentID" | tee -a $outputFile
echo "System name: $user" | tee -a $outputFile
echo "Date & Time: $dateTime" | tee -a $outputFile
echo `cat /proc/cpuinfo | grep 'vendor' | uniq` | tee -a $outputFile
echo `cat /proc/cpuinfo | grep 'model name' | uniq` | tee -a $outputFile
echo $line | tee -a $outputFile
compileTime=`date`
echo "Compiling......." | tee -a $outputFile
echo $compileTime | tee -a $outputFile
make
echo $line

echo "Executing......."
echo "Writing result into output.txt......."
echo $line | tee -a $outputFile
echo "Output: " | tee -a $outputFile
./run | tee -a $outputFile

echo $line
echo "Removing executable file: run"
make clean
