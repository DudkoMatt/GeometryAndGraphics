# 'HSL' 'HSV' 'YCbCr.601' 'YCbCr.709' 'YCoCg' 'CMY'
array=('HSL' 'HSV' 'YCbCr.601' 'YCbCr.709' 'YCoCg' 'CMY')
for i in "${array[@]}"
do
	printf "./cmake-build-debug/Lab_04.exe -f RGB -t %s -i 1 ./cmake-build-debug/file_input.ppm -o 1 ./cmake-build-debug/tmp/%s.ppm\n" "${i}" "${i}"
	./cmake-build-debug/Lab_04.exe -f RGB -t "${i}" -i 1 ./cmake-build-debug/file_input.ppm -o 1 ./cmake-build-debug/tmp/"${i}".ppm
done

for i in "${array[@]}"
do
	printf "./cmake-build-debug/Lab_04.exe -t RGB -f %s -i 1 ./cmake-build-debug/tmp/%s.ppm -o 1 ./cmake-build-debug/tmp/%s_back.ppm\n" "${i}" "${i}" "${i}"
	./cmake-build-debug/Lab_04.exe -t RGB -f "${i}" -i 1 ./cmake-build-debug/tmp/"${i}".ppm -o 1 ./cmake-build-debug/tmp/"${i}"_back.ppm
done
