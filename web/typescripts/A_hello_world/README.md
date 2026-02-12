# Install Dependences
npm install -g typescript

# Generate config files
npm init -y # generate package.json
tsc --init # generate tsconfig.json

# Start the server (Option1)
npm install -g serve
npm run start

# Start the server (Option2)
python3 -m http.server 3000
