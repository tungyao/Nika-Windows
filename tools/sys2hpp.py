def binary_to_hex(file_path):
    try:
        # 打开二进制文件
        with open(file_path, 'rb') as file:
            # 读取所有字节
            byte_data = file.read()

        # 将每个字节转换成'0x'开头的十六进制字符串
        hex_data = [f'0x{byte:02X}' for byte in byte_data]

        # 输出转换后的十六进制数据，每行16个，使用逗号分隔
        line = "static unsigned char drvrecode_eac[] = {\n"
        for i in range(0, len(hex_data), 16):
            # 使用逗号连接每行的数据
            line += '\t'+', '.join(hex_data[i:i+16]) + ',' + '\n'
        line +="};"
        with open("drvrecode_eac.hpp", 'w') as outfile:
            outfile.write(line)

    except FileNotFoundError:
        print(f"Error: The file {file_path} was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

# 示例调用函数
binary_to_hex('drvrecode_eac.sys')