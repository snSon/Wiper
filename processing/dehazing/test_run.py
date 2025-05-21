import itertools
import subprocess

flags = ['enable_haze', 'enable_aod', 'enable_roi', 'enable_blend']
combinations = list(itertools.product([False, True], repeat=4))

for idx, combo in enumerate(combinations):
    args = []
    suffix = []

    for flag, value in zip(flags, combo):
        if value:
            args.append(f'--{flag}')
            suffix.append(flag[-1])  # 마지막 문자만 따서 조합표현 (예: harb)

    suffix_str = ''.join(suffix) if suffix else 'none'
    print(f"[RUN] Case {idx+1:02d}: {suffix_str}")

    cmd = ['python3', 'processing/dehazing/hazing_and_dehazing.py',
           '--output_suffix', suffix_str] + args

    subprocess.run(cmd)
