import os
import shutil



venv = ".venv/bin/python"
full_jpg_dir = "main_driver/jpg_folder"
usb_dir = "/media"
death_star_image_dir = "main_driver/death_star_images"
detect_script = "Image_Recognition/detection.py"
cropping_script = "Image_Recognition/cropping.py"
cropped_image_dir = "main_driver/cropped_images"
encrypt_script = "encryption/encrypt.py"
decrypt_script = "encryption/decrypt.py"
tx_script = "./rk92/img_assembly_rev4/tx"
arduino_port = "/dev/ttyACM0"



def pull_jpg_files(usb_directory):
    jpg_files = []
    for root, dirs, files in os.walk(usb_directory):
        for filename in files:
            if filename.lower().endswith(".jpg"):
                jpg_files.append(os.path.join(root, filename))
    return jpg_files



def copy_jpg_files(jpg_files_list, to_directory):
    if not os.path.exists(to_directory):
        os.makedirs(to_directory)
    
    for file_path in jpg_files_list:
        filename = os.path.basename(file_path)
        destination_path = os.path.join(to_directory, filename)
        shutil.copy2(file_path, destination_path)
        print(f"Copied: {filename}")



def move_cropped_images(src_dir, dest_dir):
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    for filename in os.listdir(src_dir):
        if (filename.lower().endswith(".png") or filename.lower().endswith(".jpg")) and "cropped".lower() in filename.lower():
            src_path = os.path.join(src_dir, filename)
            dest_path = os.path.join(dest_dir, filename)
            shutil.move(src_path, dest_path)



def main():
    # Find all jpgs
    if os.path.isdir(usb_dir):
        jpg_files_list = pull_jpg_files(usb_dir)
        if jpg_files_list:
            print("jpg files found:", jpg_files_list)
        else:
            print("No jpg files found.")
    else:
        print("Invalid directory path. Please try again.")

    # Copy them to a new directory
    copy_jpg_files(jpg_files_list, full_jpg_dir)

    # Run detect script and output the ten to a new directory
    detect_command = venv + " " + detect_script + " " + death_star_image_dir
    os.system(detect_command)

    # Crop the images
    crop_command = venv + " " + cropping_script + " " + death_star_image_dir
    os.system(crop_command)

    # Move cropped images to a new directory
    move_cropped_images(death_star_image_dir, cropped_image_dir)

    # Encrpyt images
    encrypt_command = venv + " " + encrypt_script + " " + cropped_image_dir
    os.system(encrypt_command)

    # Start tx
    files = []
    full_file_string = ""
    for file in os.listdir(cropped_image_dir):
        full_file_string = full_file_string + " " + cropped_image_dir + "/" + file
        
        
    print(full_file_string)  
    tx_command = tx_script + " " + arduino_port + " " + full_file_string
    print(tx_command)
    os.system(tx_command)

    

    

if __name__ == '__main__':
    main()




