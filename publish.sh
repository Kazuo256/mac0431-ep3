#!/bin/bash

outdir=ThiagoWilson

if [ ! -d $outdir ]; then
  mkdir $outdir
fi

contents="src/ Makefile.common input* run.sh relatorio.pdf"
cp -r $contents $outdir

tar -caf $outdir.tar.gz $outdir

rm -rf $outdir

