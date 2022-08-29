# KafkaSimulation
- using library RdKafka verion 1.92

#Build KafkaSimulation

- Tạo 2 thư mục rỗng "Debug64" và "Lib64" (ngang hàng với CMakeLists.txt tổng)

- Project sử dụng các thư viện ngoài rdkafka.lib; rdkafka++.lib; OABaseD.lib; OAModelDataAPID.lib; OAUtilityD.lib nằm trong "Libraries" 
(link tải: https://ktud-my.sharepoint.com/:u:/g/personal/huyhv_ats_com_vn/ER7qiPKxysJHrkvRsY5m9kYBVKzP3nhcZk_hpiGhsqIojg?e=vj8COG ) 
Tải về giải nén rồi đặt vào Project (ngang hàng với CMakeLists.txt tổng)

- Build project bằng Cmake theo tuần tự: Lua -> ScriptModule -> KafkaSimulation

- Project sử dụng tinyxml2 (XMLParser) và nlohmann (JSON) added trong Libraries.
