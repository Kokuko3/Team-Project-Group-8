import subprocess
import os
import argparse

AES_KEY = "1234567890abcdef"  # 16-byte AES Key
AES_IV = "abcdef1234567890"   # 16-byte AES IV

# Argument parser to specify directory
parser = argparse.ArgumentParser(description='Encrypt all PNG images in a directory and replace them with .enc files.')
parser.add_argument('dir', type=str, help='Directory of images to encrypt')
args = parser.parse_args()

def encrypt_image(input_path):
    """Encrypts an image using AES-128-CBC, saves it as .enc, and deletes the original."""
    output_path = input_path.rsplit(".", 1)[0] + ".enc"
    temp_path = output_path + ".tmp" 

    cmd = f"openssl enc -aes-128-cbc -K {AES_KEY.encode().hex()} -iv {AES_IV.encode().hex()} -in {input_path} -out {temp_path}"
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error encrypting {input_path}: {result.stderr}")
        if os.path.exists(temp_path):
            os.remove(temp_path)
    else:
        os.replace(temp_path, output_path)
        os.remove(input_path)
        print(f"Encrypted: {output_path} (original deleted)")

def encrypt_all_images(folder):
    """Encrypts all PNG images in the given folder and replaces them with .enc files."""
    for filename in os.listdir(folder):
        if filename.lower().endswith(".png"):
            file_path = os.path.join(folder, filename)
            encrypt_image(file_path)

if __name__ == "__main__":
    encrypt_all_images(args.dir)



