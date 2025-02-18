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

