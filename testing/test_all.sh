#!/bin/bash

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
          ./build.sh cleanall
        else
          make cleanall
          make $1 $2 $3
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
