#!/bin/bash

#examples (run from base git respository directory!)
# ./util/test_all.sh
# ./util/test_all.sh qemu
# ./util/test_all.sh qemu debug=yes
# ./util/test_all.sh qemu debug=yes PROG_START_FUNC=run_all

ARCH=i386

for chapter in * ; do
  if [ -d $chapter ] ; then
    cd $chapter

    for increment in * ; do
      if [ -d $increment ] ; then
        cd $increment

        echo
        echo
        echo "================================================================="
        echo "Test $chapter/$increment: starting"
        echo

        if [ -e Makefile ] ; then
          make cleanall
          make "$@"
          if [ ! $? -eq 0 ] ; then
            exit
          fi
          make cleanall
        elif [ -e $ARCH/build.sh ] ; then
          cd $ARCH
          ./build.sh cleanall
          ./build.sh $1
          if [ ! $? -eq 0 ] ; then
            exit
          fi
          ./build.sh cleanall
          cd ..
        fi

        echo
        echo "Test $chapter/$increment: completed"
        echo "================================================================="
        echo
        echo
#       sleep 1
        cd ..
      fi
    done

    cd ..
  fi
done
