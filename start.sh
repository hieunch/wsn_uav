# cd Castalia/Castalia
# ./makemake
# make
# cd ../..
cd client
yarn
yarn start &
cd ..
cd Castalia/Castalia/Simulations/server
mkdir logs
mkdir trace
yarn
yarn run dev &

echo "--------------------STARTED--------------------"
