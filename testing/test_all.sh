#!/bin/bash

#examples (run from base git respository directory!)
# ./testing/test_all.sh
# ./testing/test_all.sh qemu
# ./testing/test_all.sh qemu debug=yes
# ./testing/test_all.sh qemu debug=yes PROG_START_FUNC=run_all

for chapter in * ; do
  if [ -d $chapter ] ; then
    #echo $chapter
    cd $chapter

    for increment in * ; do
      if [ -d $increment ] ; then
        cd $increment

        echo
        echo
        echo "================================================================="
        echo "Test $chapter/$increment: starting"
        echo

        if [ -e build.sh ] ; then
          ./build.sh cleanall
          ./build.sh $1
          if [ ! $? -eq 0 ] ; then
            exit
          fi
          ./build.sh cleanall
        elif [ -e Makefile ] ; then
          make cleanall
          make "$@"
          if [ ! $? -eq 0 ] ; then
            exit
          fi
          make cleanall
        fi

        echo
        echo "Test $chapter/$increment: completed"
        echo "================================================================="
        echo
        echo
#        sleep 1
        cd ..
      fi
    done

    cd ..
  fi
done
