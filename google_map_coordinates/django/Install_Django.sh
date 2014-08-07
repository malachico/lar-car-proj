#!/bin/bash
sudo python get-pip.py &&

sudo pip install Django &&

rm -r $PWD/django_projects
mkdir -p $PWD/django_projects &&

#if [[ ! -e ~/Documents/django_projects/mysite ]]; then
	(cd $PWD/django_projects && django-admin.py startproject mysite) 
#fi
#if [[ ! -e ~/Documents/django_projects/mysite/googleMapPy ]]; then
	(cd $PWD/django_projects/mysite && python manage.py startapp googleMapPy)
#fi

rm $PWD/django_projects/mysite/mysite/settings.py &&
cp $PWD/settings.py $PWD/django_projects/mysite/mysite

cp $PWD/urls.py $PWD/django_projects/mysite/googleMapPy

rm $PWD/django_projects/mysite/mysite/urls.py
cp $PWD/urls_my_site.py $PWD/django_projects/mysite/mysite/urls.py
cp $PWD/views.py $PWD/django_projects/mysite/googleMapPy/views.py

