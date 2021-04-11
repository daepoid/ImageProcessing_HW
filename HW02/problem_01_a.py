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
    return list(bmpheader), list(raw_data)


def count_number(raw_data):
    # 0부터 255까지 각 값이 몇 번 나오는지 세는 함수
    count_num_list = [0 for i in range(256)]
    for i in raw_data:
        count_num_list[i] += 1

    return count_num_list


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
        plt.hist(hist_data, bins=222)

    plt.subplot(1, 2, 2)
    image = img.imread(img_file)
    plt.imshow(image, cmap='gray')
    plt.savefig(output_path, dpi=600)
    plt.show()


def main():
    input_file = 'lena_bmp_512x512_new.bmp'
    output_path = 'outputs/problem_01_a'
    BMPHEADERS, raw_data = get_bmp_data(input_file)
    draw_graph(raw_data, input_file, output_path, True)


main()