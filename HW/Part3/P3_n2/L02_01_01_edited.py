import cherrypy
import json
import re

# http://localhost:8080/converter?value=10&originalUnit=C&targetUnit=K
@cherrypy.expose
#@cherrypy.tools.json_out()

class ConverterParams(object):
	
	keyList = ["value", "originalUnit", "targetUnit"]
	unitList = ["C", "K", "F"]

	def GET(self, **params):
		if len(params) != 3:
			raise cherrypy.HTTPError(400, "Numero parametri errato.")
		for key in params.keys():
			if key not in self.keyList:
				raise cherrypy.HTTPError(400, f"La chiave '{key}' non è valida.")

		result = self.convert(params)
		return json.dumps(result)
		#oppure return result inserendo @cherrypy.tools.json_out()

	def convert(self, dict):
		if re.fullmatch("-?[0-9]+\.?[0-9]+", dict["value"])==None:
			raise cherrypy.HTTPError(400, f"Errore tipo parametri: valore da convertire non riconosciuto.")
		value = float(dict["value"])
		originalUnit = dict["originalUnit"]
		targetUnit =  dict["targetUnit"]

		if originalUnit not in self.unitList:
			raise cherrypy.HTTPError(400, f"Il valore '{originalUnit}' non è valido.")
		if targetUnit not in self.unitList:
			raise cherrypy.HTTPError(400, f"Il valore '{targetUnit}' non è valido.")
		
		intermediateValue = value
		#converto tutto in K e dopo nell'unità finale
		if originalUnit == "C":
			intermediateValue = value + 273.15
		if originalUnit == "F":
			intermediateValue = (value + 459.67)*5/9

		finalValue = intermediateValue

		if targetUnit == "C":
			finalValue = intermediateValue - 273.15
		if targetUnit == "F":
			finalValue = intermediateValue * 9/5 - 459.67

		result = {"originalValue" : round(value, 5),
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

	cherrypy.tree.mount(ConverterParams(),'/converter',conf)
	cherrypy.tree.mount(fromArduino(),'/log',conf)
	#da modificare ogni volta che cambia l'ip del computer
	cherrypy.config.update({'server.socket_host':'192.168.43.40'})
	cherrypy.config.update({'server.socket_port': 8080})
	cherrypy.engine.start()
	cherrypy.engine.block()