from flask import Flask, render_template, send_from_directory, url_for
import os

app = Flask(__name__, template_folder=".")


IMAGE_FOLDER = os.path.abspath("/home/mmcdaniel/git/Team-Project-Group-8/rk92/img_assembly_rev4")

@app.route('/')
def gallery():
    image_files = [f for f in os.listdir(IMAGE_FOLDER) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif'))]
    return render_template("gallery2.html", images=image_files)

@app.route('/images/<filename>')
def serve_image(filename):
    return send_from_directory(IMAGE_FOLDER, filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)

