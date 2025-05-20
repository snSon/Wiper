import numpy as np
from scipy.ndimage import gaussian_filter
from skimage.util import dtype_range

def ssim(A, ref, radius=1.5, dynamic_range=None, regularization_constants=None, exponents=(1, 1, 1)):
    A = A.astype(np.float64)
    ref = ref.astype(np.float64)

    if A.shape != ref.shape:
        raise ValueError("Input images must have the same dimensions.")
    if A.ndim > 3:
        raise ValueError("Images with more than 3 dimensions are not supported.")

    if dynamic_range is None:
        dynamic_range = dtype_range[A.dtype.type][1] if A.dtype.kind != 'f' else 1.0

    if regularization_constants is None:
        C1 = (0.01 * dynamic_range) ** 2
        C2 = (0.03 * dynamic_range) ** 2
        C3 = C2 / 2
    else:
        C1, C2, C3 = regularization_constants

    alpha, beta, gamma = exponents

    # Gaussian filter
    def filter_fn(img):
        return gaussian_filter(img, sigma=radius, mode='reflect')

    mu_x = filter_fn(A)
    mu_y = filter_fn(ref)
    mu_x_sq = mu_x ** 2
    mu_y_sq = mu_y ** 2
    mu_xy = mu_x * mu_y

    sigma_x_sq = filter_fn(A * A) - mu_x_sq
    sigma_y_sq = filter_fn(ref * ref) - mu_y_sq
    sigma_xy = filter_fn(A * ref) - mu_xy

    # Special case: default exponents and C3 = C2/2
    if C3 == C2 / 2 and alpha == beta == gamma == 1:
        num = (2 * mu_xy + C1) * (2 * sigma_xy + C2)
        den = (mu_x_sq + mu_y_sq + C1) * (sigma_x_sq + sigma_y_sq + C2)
        ssim_map = np.ones_like(A)
        valid = den != 0
        ssim_map[valid] = num[valid] / den[valid]
    else:
        # General case
        ssim_map = np.ones_like(A)

        # Luminance term
        if alpha > 0:
            l_num = 2 * mu_xy + C1
            l_den = mu_x_sq + mu_y_sq + C1
            l_term = np.ones_like(A)
            valid = l_den != 0
            l_term[valid] = l_num[valid] / l_den[valid]
            ssim_map *= l_term ** alpha

        # Contrast term
        sigmaxsigmay = None
        if beta > 0:
            sigmaxsigmay = np.sqrt(sigma_x_sq * sigma_y_sq)
            c_num = 2 * sigmaxsigmay + C2
            c_den = sigma_x_sq + sigma_y_sq + C2
            c_term = np.ones_like(A)
            valid = c_den != 0
            c_term[valid] = c_num[valid] / c_den[valid]
            ssim_map *= c_term ** beta

        # Structure term
        if gamma > 0:
            if sigmaxsigmay is None:
                sigmaxsigmay = np.sqrt(sigma_x_sq * sigma_y_sq)
            s_num = sigma_xy + C3
            s_den = sigmaxsigmay + C3
            s_term = np.ones_like(A)
            valid = s_den != 0
            s_term[valid] = s_num[valid] / s_den[valid]
            ssim_map *= s_term ** gamma

    ssim_val = np.mean(ssim_map)
    return ssim_val, ssim_map
