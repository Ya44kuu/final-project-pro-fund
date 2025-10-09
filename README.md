"# final-project-pro-fund" 

หากโหลดเเล้ว compile ตามไม่ได้ให้ลองเช็คก่อนว่าเข้าถึง path ที่ไฟล์อยุ่เเล้วรึยัง โดย

ทำใน terminal vs code
       
cd /d "ที่อยุ่ของไฟล์"


ex.   cd /d "E:\final\code"

//compile code//  Windows



gcc -std=c11 -Wall -Wextra -O2 final.c unit_tests.c e2e_tests.c -o booking_app.exe


macOS / Linux//              

gcc -std=c11 -Wall -Wextra -O2 final.c unit_tests.c e2e_tests.c -o booking_app



//run//  
booking_app.exe  (window)



./booking_app  (mac/linux)
