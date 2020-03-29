#!/usr/bin/env python


import requests
import time


while(1):
	ip = requests.get("https://api.ipify.org").text

	payload={
		"myip":ip,
		"system":"dyndns",
		"hostname":"podometre.geraudfrancois.fr"
	}

	headers={
		"user-agent":"ovh-dynhost"
	}

	authentication = requests.auth.HTTPBasicAuth("geraudfrancois.fr-gefrancois", "pelitopelito")
	request = requests.Request('GET', url="https://www.ovh.com/nic/update", params=payload,headers=headers, auth=authentication).prepare()
	date = time.gmtime()
	try:
		session = requests.Session()
		response = session.send(request).text.lower()
		print(str(date.tm_hour)+":"+str(date.tm_min)+":"+str(date.tm_sec)+" "+str(date.tm_mday)+":"+str(date.tm_mon)+":"+str(date.tm_year)+" "+response+"\n")
	except:
		print(str(date.tm_hour)+":"+str(date.tm_min)+":"+str(date.tm_sec)+" "+str(date.tm_mday)+":"+str(date.tm_mon)+":"+str(date.tm_year)+"Erreur dans lors de l'envoi de la requete !")	
	time.sleep(300)

