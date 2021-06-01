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

    output_path += '_bar.png'
    count_num_list = count_number(hist_data)
    x = np.arange(256)
    y = np.array(count_num_list)
    plt.bar(x, y)

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


def nint(x):
    if x < 0:
        return int(x - 0.5)
    else:
        return int(x + 0.5)


def DCT(ix):
    global B_SIZE
    x = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]
    z = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]
    y = [0 for val in range(B_SIZE)]
    yy = [0 for val in range(B_SIZE)]
    c = [0 for val in range(40)]
    s = [0 for val in range(40)]
    ft = [0, 0, 0, 0]
    fxy = [0, 0, 0, 0]
    zz = 0

    for i in range(40):
        zz = math.pi * float(i + 1) / 64.0
        c[i] = math.cos(zz)
        s[i] = math.sin(zz)

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            y[jj] = z[jj][ii]

        for jj in range(4):
            ft[jj] = y[jj] + y[7 - jj]

        fxy[0] = ft[0] + ft[3]
        fxy[1] = ft[1] + ft[2]
        fxy[2] = ft[1] - ft[2]
        fxy[3] = ft[0] - ft[3]

        ft[0] = c[15] * (fxy[0] + fxy[1])
        ft[2] = c[15] * (fxy[0] - fxy[1])
        ft[1] = s[7] * fxy[2] + c[7] * fxy[3]
        ft[3] = -s[23] * fxy[2] + c[23] * fxy[3]

        for jj in range(4, 8):
            y[jj] = y[7 - jj] - y[jj]

        y[4] = yy[4]
        y[7] = yy[7]
        y[5] = c[15] * (-yy[5] + yy[6])
        y[6] = c[15] * (yy[5] + yy[6])

        yy[4] = y[4] + y[5]
        yy[5] = y[4] - y[5]
        yy[6] = -y[6] + y[7]
        yy[7] = y[6] + y[7]

        y[0] = ft[0]
        y[4] = ft[2]
        y[2] = ft[1]
        y[6] = ft[3]
        y[1] = s[3] * yy[4] + c[3] * yy[7]
        y[5] = s[19] * yy[5] + c[19] * yy[6]
        y[3] = -s[11] * yy[5] + c[11] * yy[6]
        y[7] = -s[27] * yy[4] + c[27] * yy[7]

        for jj in range(B_SIZE):
            z[ii][jj] = y[jj]

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            y[jj] = z[jj][ii]

        for jj in range(4):
            ft[jj] = y[jj] + y[7 - jj]

        fxy[0] = ft[0] + ft[3]
        fxy[1] = ft[1] + ft[2]
        fxy[2] = ft[1] - ft[2]
        fxy[3] = ft[0] - ft[3]

        ft[0] = c[15] * (fxy[0] + fxy[1])
        ft[2] = c[15] * (fxy[0] - fxy[1])
        ft[1] = s[7] * fxy[2] + c[7] * fxy[3]
        ft[3] = -s[23] * fxy[2] + c[23] * fxy[3]

        for jj in range(4, 8):
            yy[jj] = y[7 - jj] - y[jj]

        y[4] = yy[4]
        y[7] = yy[7]
        y[5] = c[15] * (-yy[5] + yy[6])
        y[6] = c[15] * (yy[5] + yy[6])

        yy[4] = y[4] + y[5]
        yy[5] = y[4] - y[5]
        yy[6] = -y[6] + y[7]
        yy[7] = y[6] + y[7]

        y[0] = ft[0]
        y[4] = ft[2]
        y[2] = ft[1]
        y[6] = ft[3]
        y[1] = s[3] * yy[4] + c[3] * yy[7]
        y[5] = s[19] * yy[5] + c[19] * yy[6]

        y[3] = -s[11] * yy[5] + c[11] * yy[6]
        y[7] = -s[27] * yy[4] + c[27] * yy[7]

        for jj in range(B_SIZE):
            y[jj] = y[jj] / 4.0

        for jj in range(B_SIZE):
            z[jj][ii] = y[jj]

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            ix[ii][jj] = nint(z[ii][jj])


def IDCT(ix):
    global B_SIZE
    x = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]
    z = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]
    y = [0 for val in range(B_SIZE)]
    yy = [0 for val in range(B_SIZE)]
    c = [0 for val in range(40)]
    s = [0 for val in range(40)]
    ait = [0, 0, 0, 0]
    aixy = [0, 0, 0, 0]
    zz = 0

    for i in range(40):
        zz = math.pi * float(i + 1) / 64.0
        c[i] = math.cos(zz)
        s[i] = math.sin(zz)

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            x[ii][jj] = float(ix[ii][jj])

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            y[jj] = x[jj][ii]

        ait[0] = y[0]
        ait[1] = y[2]
        ait[2] = y[4]
        ait[3] = y[6]

        aixy[0] = c[15] * (ait[0] + ait[2])
        aixy[1] = c[15] * (ait[0] - ait[2])
        aixy[2] = s[7] * ait[1] - s[23] * ait[3]
        aixy[3] = c[7] * ait[1] + c[23] * ait[3]

        ait[0] = aixy[0] + aixy[3]
        ait[1] = aixy[1] + aixy[2]
        ait[2] = aixy[1] - aixy[2]
        ait[3] = aixy[0] - aixy[3]

        yy[4] = s[3] * y[1] - s[27] * y[7]
        yy[5] = s[19] * y[5] - s[11] * y[3]
        yy[6] = c[19] * y[5] + c[11] * y[3]
        yy[7] = c[3] * y[1] + c[27] * y[7]

        y[4] = yy[4] + yy[5]
        y[5] = yy[4] - yy[5]
        y[6] = -yy[6] + yy[7]
        y[7] = yy[6] + yy[7]

        yy[4] = y[4]
        yy[7] = y[7]
        yy[5] = c[15] * (-y[5] + y[6])
        yy[6] = c[15] * (y[5] + y[6])

        for jj in range(4):
            y[jj] = ait[jj] + yy[7 - jj]

        for jj in range(4, 8):
            y[jj] = ait[7 - jj] - yy[jj]

        for jj in range(B_SIZE):
            z[jj][ii] = y[jj]

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            y[jj] = z[ii][jj]

        ait[0] = y[0]
        ait[1] = y[2]
        ait[2] = y[4]
        ait[3] = y[6]

        aixy[0] = c[15] * (ait[0] + ait[2])
        aixy[1] = c[15] * (ait[0] - ait[2])
        aixy[2] = s[7] * ait[1] - s[23] * ait[3]
        aixy[3] = c[7] * ait[1] + c[23] * ait[3]

        ait[0] = aixy[0] + aixy[3]
        ait[1] = aixy[1] + aixy[2]
        ait[2] = aixy[1] - aixy[2]
        ait[3] = aixy[0] - aixy[3]

        yy[4] = s[3] * y[1] - s[27] * y[7]
        yy[5] = s[19] * y[5] - s[11] * y[3]
        yy[6] = c[19] * y[5] + c[11] * y[3]
        yy[7] = c[3] * y[1] + c[27] * y[7]

        y[4] = yy[4] + yy[5]
        y[5] = yy[4] - yy[5]
        y[6] = -yy[6] + yy[7]
        y[7] = yy[6] + yy[7]

        yy[4] = y[4]
        yy[7] = y[7]
        yy[5] = c[15] * (-y[5] + y[6])
        yy[6] = c[15] * (y[5] + y[6])

        for jj in range(4):
            y[jj] = ait[jj] + yy[7 - jj]

        for jj in range(4, 8):
            y[jj] = ait[7 - jj] - yy[jj]

        for jj in range(B_SIZE):
            z[ii][jj] = y[jj] / 4.0

    for ii in range(B_SIZE):
        for jj in range(B_SIZE):
            ix[ii][jj] = nint(z[ii][jj])


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

    DCT_image = [0 for val in range(MATRIX_MAX * MATRIX_MAX)]
    restored_image = [0 for val in range(MATRIX_MAX * MATRIX_MAX)]

    print("DCT start")

    for i in range(int(MATRIX_MAX / B_SIZE)):
        for j in range(int(MATRIX_MAX / B_SIZE)):
            copied = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]

            for a in range(i * B_SIZE, i * B_SIZE + B_SIZE):
                for b in range(j * B_SIZE, j * B_SIZE + B_SIZE):
                    copied[a - (i * B_SIZE)][b -
                                             (j * B_SIZE)] = raw_image[MATRIX_MAX * a + b]

            DCT(copied)

            for a in range(i * B_SIZE, i * B_SIZE + B_SIZE):
                for b in range(j * B_SIZE, j * B_SIZE + B_SIZE):
                    DCT_image[MATRIX_MAX * a + b] = copied[a -
                                                           (i * B_SIZE)][b - (j * B_SIZE)]

    for i in range(int(MATRIX_MAX / B_SIZE)):
        for j in range(int(MATRIX_MAX / B_SIZE)):
            copied = [[0 for row in range(B_SIZE)] for col in range(B_SIZE)]

            for a in range(i * B_SIZE, i * B_SIZE + B_SIZE):
                for b in range(j * B_SIZE, j * B_SIZE + B_SIZE):
                    copied[a - (i * B_SIZE)][b - (j * B_SIZE)
                                             ] = DCT_image[MATRIX_MAX * a + b]

            IDCT(copied)

            for a in range(i * B_SIZE, i * B_SIZE + B_SIZE):
                for b in range(j * B_SIZE, j * B_SIZE + B_SIZE):
                    restored_image[MATRIX_MAX * a +
                                   b] = copied[a - (i * B_SIZE)][b - (j * B_SIZE)]

    # DCT_image = DCT(raw_image)
    print("DCT clear")
    print("IDCT start")
    # restored_image = IDCT(DCT_image)
    print("IDCT clear")
    mse = MSE(raw_image, restored_image)
    print(mse)

    create_bmp_img(BMPHEADERS, restored_image, new_img_path)


main()
