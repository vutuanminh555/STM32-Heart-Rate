# STM32-Heart-Rate
* Hệ thống đo nhịp tim sử dụng phương pháp áp lực tĩnh mạch đồ hồng ngoại (Photoplethysmography)
* Cơ chế hoạt động:

  ![image](https://github.com/user-attachments/assets/dea95c62-7c00-4ebd-8d2c-8e97d1275e73)
  
  - Cảm biến hoạt động bằng cách chiếu cặp LED vào vùng da mỏng (ngón tay hoặc dái tai) và đo lượng ánh sáng bị phản chiếu ngược trở lại sử dụng cảm biến quang. Hemoglobin có chứa oxy trong động mạch có tính hấp thụ ánh sáng hồng ngoại. Máu càng đỏ (hàm lượng hemoglobin càng lớn), lượng ánh sáng hồng ngoại bị hấp thụ càng nhiều. Máu được bơm qua mạch máu bởi nhịp đập của tim sẽ tạo ra hàm đồ thị thay đổi theo thời gian ở đầu ra của cảm biến quang. Qua đó ta có thể tính toán được nhịp đập của tim.

  ![image](https://github.com/user-attachments/assets/53c8fa13-ee59-4334-91c9-bf2cb2053493)
  
  - Bên cạnh nhịp tim, lượng ánh sáng phản xạ từ cảm biến quang còn có thể được sử dụng để thu thập thông tin về nồng độ oxy trong máu. Đồ thị bên dưới thể hiện mối quan hệ giữa hemoglobin có chứa oxy (HbO2) và không chứa oxy (Hb).

  ![image](https://github.com/user-attachments/assets/00178108-e62b-4cef-acb9-ffed52bfb681)
  
  - Từ đồ thị trên, ta có thể thấy máu thiếu oxy hấp thụ nhiều ánh sáng đỏ (660nm), trong khi đó máu nhiều oxy hấp thụ nhiều ánh sáng hồng ngoại (880nm). Bằng việc tính toán tỷ lệ giữa ánh sáng đỏ và ánh sáng hồng ngoại được phản chiếu ngược trở lại cảm biến quang, ta có thể tìm được nồng độ oxy trong máu.
* Kết quả sản phẩm:

  ![image](https://github.com/user-attachments/assets/41dbb09f-6579-4e97-bc41-f57ed9fe7888)
  
  - Quá trình lấy mẫu tín hiệu:
    
  ![image](https://github.com/user-attachments/assets/fb8752fe-fc49-45a1-b01d-512b5786a2d0)
  
  - Đo tín hiệu thành công:
    
  ![image](https://github.com/user-attachments/assets/8c4b18b4-c99d-4de6-882b-ec2942f4b5e8)
  
  - Qua đo đạc, sản phẩm nguyên mẫu mất khoảng 5s để lấy mẫu tín hiệu thành công. Mức điện áp tiêu thụ là 3.3V, dòng tiêu thụ là 50mA.
* Kiểm chứng sản phẩm với phương pháp đo nhịp tim sử dụng cùng công nghệ Photoplethysmography dùng cảm biến hồng ngoại trên điện thoại Galaxy S9:
  - Thông số sản phẩm đo được:
    
  ![image](https://github.com/user-attachments/assets/6e0ef704-7f07-4a44-8211-5f982c466265)
  
  - Thông số Galaxy S9 đo được:
    
  ![image](https://github.com/user-attachments/assets/16fbc4c5-a70b-4ed6-9e6c-c33683a54041)
  
 

    





  
