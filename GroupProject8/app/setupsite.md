# Setting up the Web Server with Static IP

## Chase Ennis and Cade Wrinkle

### Configuring Apache

1. Install Apache
    * `sudo apt update`
    * `sudo apt install apache2`
    * `sudo ufw allow ssh`
    * `sudo ufw allow 'Apache'`
    * `sudo ufw status` (if inactive run `sudo ufw enable`) 
    * `ufw rules` (to verify the rules)
    * `sudo systemctl status apache2` (should be up and running)
2. Moving base `site.tar.gz` to instance
    * Site can be found [basesite](./basesite/)
    * `sudo scp -i .ssh/CEG4480.pem -r Downloads/site.tar.gz ubuntu@54.156.255.214:/home/ubuntu/` (command to move the file from local system to AWS instance)
    * `tar -xvzf site.tar.gz` (extract file)
3. Host Site Content
    * Move all the contents into `/var/www/html/`: `sudo mv assets/ email.html headers.css  index.html /var/www/html`
    * Enter DocumentRoot File: `sudo vim /etc/apache2/sites-available/000-default.conf`
    * Ensure `DocumentRoot` says `DocumentRoot /var/www/html`
    * Curl it to verify it works: ` curl -I http://54.156.255.214`
    * Type into the browser and it will be there
      
