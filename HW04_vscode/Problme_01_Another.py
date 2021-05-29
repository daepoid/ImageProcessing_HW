import struct
import math
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as img

MATRIX_MAX = 512
B_SIZE = 8


def draw_graph(hist_data, img_file, output_path, mode=True):
    plt.figure(figsize=(16, 9))
    plt.subplot(1, 2, 1)
    if mode:
        output_path += '_bar.png'
        count_num_list = count_number(hist_data)
        x = np.arange(256)
        y = np.array(count_num_list)
        plt.bar(x, y)
    else:
        output_path += '_hist.png'
        plt.hist(hist_data, bins=222)

    plt.subplot(1, 2, 2)
    image = img.imread(img_file)
    plt.imshow(image, cmap='gray')
    plt.savefig(output_path, dpi=600)
    plt.show()


def read_bmp_img(filepath):
    bmpheader = list()
    raw_image = list()
    with open(filepath, 'rb') as bmp:
        bmpheader = struct.unpack('B' * 1078, bmp.read(1078))
        raw_image = struct.unpack('B' * 512 * 512, bmp.read(512 * 512))
    return list(bmpheader), list(raw_image)


def create_bmp_img(BMPHEADERS, raw_image, new_img_path):
    with open(new_img_path, 'wb') as bmp:
        for i in range(len(BMPHEADERS)):
            bmp.write(struct.pack('B', BMPHEADERS[i]))

        for i in range(len(raw_image)):
            bmp.write(struct.pack('B', raw_image[i]))


def DCT(input_image):
    global MATRIX_MAX, B_SIZE
    STEP = int(MATRIX_MAX * MATRIX_MAX / (B_SIZE * B_SIZE))

    DCT_image = [0.0 for val in range(MATRIX_MAX * MATRIX_MAX)]

    for s in range(STEP):
        matrix = [[0 for col in range(B_SIZE)] for row in range(B_SIZE)]
        for i in range(B_SIZE):
            for j in range(B_SIZE):
                matrix[i][j] = input_image[j +
                                           B_SIZE * i + B_SIZE * B_SIZE * s]

        for i in range(B_SIZE):
            for j in range(B_SIZE):
                sum = 0.0
                ci = 0
                cj = 0
                if i == 0:
                    ci = 1 / math.sqrt(2)
                else:
                    ci = 1

                if j == 0:
                    cj = 1 / math.sqrt(2)
                else:
                    cj = 1

                for k in range(B_SIZE):
                    for l in range(B_SIZE):
                        dct_val = matrix[k][l] * \
                            math.cos((2 * k + 1) * i * math.pi / (2 * B_SIZE)) * \
                            math.cos((2 * l + 1) * j * math.pi / (2 * B_SIZE))

                DCT_image[j + B_SIZE * i + B_SIZE * B_SIZE *
                          s] = (2 * ci * cj * sum / math.sqrt(B_SIZE * B_SIZE))

    return DCT_image


def IDCT(DCT_image):
    global MATRIX_MAX, B_SIZE
    STEP = int(MATRIX_MAX * MATRIX_MAX / (B_SIZE * B_SIZE))

    output_image = [0 for val in range(MATRIX_MAX * MATRIX_MAX)]

    for s in range(STEP):
        matrix = [[0 for col in range(B_SIZE)] for row in range(B_SIZE)]
        for i in range(B_SIZE):
            for j in range(B_SIZE):
                matrix[i][j] = DCT_image[j + B_SIZE * i + B_SIZE * B_SIZE * s]

        for i in range(B_SIZE):
            for j in range(B_SIZE):
                sum = 0.0
                ci = 0
                cj = 0
                for k in range(B_SIZE):
                    for l in range(B_SIZE):
                        if k == 0:
                            ci = 1 / math.sqrt(2)
                        else:
                            ci = 1

                        if l == 0:
                            cj = 1 / math.sqrt(2)
                        else:
                            cj = 1

                        dct_val = cj * ci / 4 * matrix[k][l] * math.cos((2 * i + 1) * k * math.pi / (
                            2 * B_SIZE)) * math.cos((2 * j + 1) * l * math.pi / (2 * B_SIZE))
                        sum += dct_val

                if sum > 255:
                    sum = 255
                elif sum < 0:
                    sum = 0
                else:
                    sum = int(sum)

                output_image[j + B_SIZE * i + B_SIZE * B_SIZE * s] = sum

    return output_image


def MSE(original_image, restored_image):
    global MATRIX_MAX
    temp = 0.0

    for i in range(MATRIX_MAX * MATRIX_MAX):
        temp += (original_image[i] - restored_image[i])**2

    return temp / (MATRIX_MAX * MATRIX_MAX)


def main():
    input_file = 'lena_bmp_512x512_new.bmp'
    output_graph_path = 'outputs/problem_01_a'
    new_img_path = 'outputs/Restored_DCT_lena_py.bmp'

    BMPHEADERS, raw_image = read_bmp_img(input_file)

    print("DCT start")
    DCT_image = DCT(raw_image)
    print("DCT clear")
    print("IDCT start")
    restored_image = IDCT(DCT_image)
    print("IDCT clear")
    mse = MSE(raw_image, restored_image)
    print(mse)

    create_bmp_img(BMPHEADERS, restored_image, new_img_path)
    # draw_graph(hist_data, new_img_path, output_graph_path, True)


main()
