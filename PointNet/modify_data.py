import os
import numpy as np
from scipy.spatial.transform import Rotation
import glob

def modify_subset1():
    # Subset 1: drop dimension
    base_dir = '../PointNet/shapenet-core-seg-less-subset1/Shapenetcore_benchmark/'
    points_files = glob.glob(os.path.join(base_dir, '*', 'points', '*.npy'))
    for file_path in points_files:
        pc_data = np.load(file_path)
        axis = np.random.randint(0, 3)
        pc_data[:, axis] = 0
        np.save(file_path, pc_data)

def modify_subset2():
    base_dir = '../PointNet/shapenet-core-seg-less-subset2/Shapenetcore_benchmark/'
    points_files = glob.glob(os.path.join(base_dir, '*', 'points', '*.npy'))
    cam_dist = 2.5
    eps = 1e-6

    for file_path in points_files:
        pc_data = np.load(file_path)

        rot = Rotation.random().as_matrix()
        pc_data = pc_data @ rot

        pc_data[:, 2] += cam_dist

        X = pc_data[:, 0]
        Y = pc_data[:, 1]
        Z = pc_data[:, 2] + eps

        x_proj = X / Z
        y_proj = Y / Z

        pc_proj = np.stack([x_proj, y_proj, np.zeros_like(Z)], axis=1)

        np.save(file_path, pc_proj)


if __name__ == '__main__':
    modify_subset1()
    modify_subset2()
