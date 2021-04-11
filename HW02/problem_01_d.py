import struct
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as img


def get_bmp_data(filepath):
    bmpheader = list()
    raw_data = list()
    with open(filepath, 'rb') as bmp:
        bmpheader = struct.unpack('B' * 1078, bmp.read(1078))
        raw_data = struct.unpack('B' * 512 * 512, bmp.read(512 * 512))
    return bmpheader, raw_data


def count_number(raw_data):
    # 0부터 255까지 각 값이 몇 번 나오는지 세는 함수
    count_num_list = [0 for i in range(256)]
    for i in raw_data:
        count_num_list[i] += 1

    return count_num_list


def create_bmp_img(BMPHEADERS, raw_data, new_img_path):
    with open(new_img_path, 'wb') as bmp:
        for i in range(len(BMPHEADERS)):
            bmp.write(struct.pack('B', BMPHEADERS[i]))
        
        for i in range(len(raw_data)):
            bmp.write(struct.pack('B', raw_data[i]))
            

def draw_graph(hist_data, img_file, output_path, mode = True):
    plt.figure(figsize=(16, 9))
    plt.subplot(1, 2, 1)
    if mode :
        output_path += '_bar.png'
        count_num_list = count_number(hist_data)
        x = np.arange(256)
        y = np.array(count_num_list)
        plt.bar(x, y)
    else :
        output_path += '_hist.png'
        plt.hist(hist_data, bins=255)

    plt.subplot(1, 2, 2)
    image = img.imread(img_file)
    plt.imshow(image, cmap='gray')
    plt.savefig(output_path, dpi=600)
    plt.show()


def endsin_contrast_stretching(raw_data):
    hist_data = [0 for i in range(len(raw_data))]
    min_val = 50
    max_val = 190

    for i in range(len(raw_data)):
        temp = int((raw_data[i] - min_val) / (max_val - min_val) * 255)
        if temp > 255:
            hist_data[i] = 255
        elif temp < 0:
            hist_data[i] = 0            
        else:
            hist_data[i] = temp
    
    return hist_data


def main():
    input_file = 'lena_bmp_512x512_new.bmp'
    output_graph_path = 'outputs/problem_01_d'
    new_img_path = 'outputs/d_endsin_contrast_stretching_lena.bmp'

    BMPHEADERS, raw_data = get_bmp_data(input_file)
    hist_data = endsin_contrast_stretching(raw_data)

    create_bmp_img(BMPHEADERS, hist_data, new_img_path)
    draw_graph(hist_data, new_img_path, output_graph_path, True)


main()
