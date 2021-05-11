#!/bin/bash

DEPENDENCIES=$(cat <<-END
  wiringpi
  libboost-all-dev
END
)

FLAGS=$(cat <<-END
 -y
 --fix-missing
END
)

sudo apt install $DEPENDENCIES $FLAGS
