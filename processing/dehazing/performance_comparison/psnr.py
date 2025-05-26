import numpy as np
import warnings

def psnr(A, ref, peakval=None):
    """
    PSNR(A, ref, peakval=None)
    Calculate the Peak Signal-to-Noise Ratio (PSNR) and Signal-to-Noise Ratio (SNR)
    between two images A and ref.

    Parameters:
    -----------
    A : ndarray
        Test image (same shape and dtype as ref).
    ref : ndarray
        Reference image.
    peakval : float, optional
        Maximum possible pixel value. If not given, inferred from image dtype.

    Returns:
    --------
    peaksnr : float
        Peak Signal-to-Noise Ratio in dB.
    snr : float
        Signal-to-Noise Ratio in dB.
    """

    check_images(A, ref)

    if peakval is None:
        peakval = get_range_from_dtype(A)
    else:
        check_peakval(peakval, A)

    if A.size == 0:
        return 0.0, 0.0

    mse = np.mean((A.astype(np.float64) - ref.astype(np.float64)) ** 2)

    if mse == 0:
        return float('inf'), float('inf')

    peaksnr = 10 * np.log10((peakval ** 2) / mse)

    ref_power = np.mean(ref.astype(np.float64) ** 2)
    snr = 10 * np.log10(ref_power / mse) if ref_power > 0 else float('-inf')

    return peaksnr, snr


def check_images(A, ref):
    if A.dtype != ref.dtype:
        raise ValueError("Input images A and REF must be of the same dtype.")
    if A.shape != ref.shape:
        raise ValueError("Input images A and REF must have the same shape.")
    if not A.dtype.name in ['uint8', 'uint16', 'int16', 'float32', 'float64']:
        raise TypeError("Unsupported image dtype. Must be uint8, uint16, int16, float32, or float64.")


def check_peakval(peakval, A):
    if not np.isscalar(peakval) or peakval < 0:
        raise ValueError("PEAKVAL must be a non-negative scalar.")
    max_possible = get_range_from_dtype(A)
    if np.issubdtype(A.dtype, np.integer) and peakval > max_possible:
        warnings.warn(f"PEAKVAL ({peakval}) is greater than the maximum possible value for dtype {A.dtype}.")


def get_range_from_dtype(img):
    """Return the dynamic range based on the image dtype."""
    if img.dtype == np.uint8:
        return 255
    elif img.dtype == np.uint16:
        return 65535
    elif img.dtype == np.int16:
        return 32767
    elif img.dtype == np.float32 or img.dtype == np.float64:
        return 1.0
    else:
        raise TypeError("Unsupported dtype for range detection.")

