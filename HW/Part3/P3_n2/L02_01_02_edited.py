import cherrypy
import json
import re

class ConverterUri(object):
	exposed=True
	unitList=["C","K","F"]

	def GET(self, *uri):
		if len(uri) != 3:
			raise cherrypy.HTTPError(400, "Numero parametri errato.")
		result=self.convert(uri)
		return json.dumps(result)

	def convert(self, uris):
		if re.fullmatch("-?[0-9]+\.?[0-9]+", uris[0])!=None:
			value=float(uris[0])
			originalUnit=uris[1]
			targetUnit=uris[2]
		elif re.fullmatch("-?[0-9]+\.?[0-9]+", uris[1])!=None:
			originalUnit=uris[0]
			value=float(uris[1])
			targetUnit=uris[2]
		elif re.fullmatch("-?[0-9]+\.?[0-9]+", uris[2])!=None:
			originalUnit=uris[0]
			targetUnit=uris[1]
			value=float(uris[2])
		else:
			raise cherrypy.HTTPError(400, "Errore tipo parametri: valore da convertire non riconosciuto.")

		if originalUnit not in self.unitList:
			raise cherrypy.HTTPError(400, f"Il valore {originalUnit} non è valido.")
		if targetUnit not in self.unitList:
			raise cherrypy.HTTPError(400, f"Il valore {targetUnit} non è valido.")

		finalValue=value

		if originalUnit=="C":
			finalValue=value+273.15
		elif originalUnit=="F":
			finalValue=(value+459.67)*5/9

		if targetUnit=="C":
			finalValue-=273.15
		elif targetUnit=="F":
			finalValue=finalValue*9/5-459.67

		result={"originalValue" : round(value, 5),
				"originalUnit" : originalUnit,
				"finalValue" : round(finalValue, 5),
				"finalUnit" : targetUnit}
		return result

@cherrypy.expose
class fromArduino(object):
	listTemp = []

	def GET(self):
		#ritorno lista come json
		if len(self.listTemp) == 0:
			return "Empty list\n"
		return json.dumps(self.listTemp)
		
	def POST(self):
		#catturo il json dal body e lo converto in dict
		strBody = ""
		strBody = cherrypy.request.body.read()
		try:
			jsDictionary = json.loads(strBody)
			#inserisco dizionario nella lista
			self.listTemp.append(jsDictionary)
		except ValueError as e:
			raise cherrypy.HTTPError(400, f"Json inviato non valido")
		return

if __name__ == '__main__':
	conf={
		'/':{
				'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
				'tool.session.on':True,
	
		},
		
	}

	cherrypy.tree.mount(ConverterUri(),'/converter',conf)
	cherrypy.tree.mount(fromArduino(),'/log',conf)
	#da modificare ogni volta che cambia l'ip del computer
	cherrypy.config.update({'server.socket_host':'192.168.43.40'})
	cherrypy.config.update({'server.socket_port': 8080})
	cherrypy.engine.start()
	cherrypy.engine.block()