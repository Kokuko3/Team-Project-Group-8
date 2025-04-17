import subprocess
import os
import argparse

AES_KEY = "1234567890abcdef"  # 16-byte AES Key
AES_IV = "abcdef1234567890"   # 16-byte AES IV

# Argument parser to specify directory
parser = argparse.ArgumentParser(description='Decrypt all .enc images in a directory, overwriting them.')
parser.add_argument('dir', type=str, help='Directory of encrypted images to decrypt in-place')
args = parser.parse_args()

def decrypt_image(input_path):
    """Decrypts an image using AES-128-CBC and overwrites the original .enc file."""
    output_path = input_path.replace(".enc", "")
    temp_path = output_path + ".tmp"  # Temporary path to avoid corruption
    cmd = f"openssl enc -d -aes-128-cbc -K {AES_KEY.encode().hex()} -iv {AES_IV.encode().hex()} -in {input_path} -out {temp_path}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error decrypting {input_path}: {result.stderr}")
        if os.path.exists(temp_path):
            os.remove(temp_path)
    else:
        os.replace(temp_path, output_path)
        os.remove(input_path)  # Remove the original .enc file
        print(f"Decrypted and replaced: {output_path}")


def decrypt_all_images(folder):
    """Decrypts all .enc files in the given folder and overwrites them in-place."""
    for filename in os.listdir(folder):
        if filename.endswith(".enc"):
            file_path = os.path.join(folder, filename)
            decrypt_image(file_path)

if __name__ == "__main__":
    decrypt_all_images(args.dir)


