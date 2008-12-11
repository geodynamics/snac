#!/usr/bin/env python
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# <LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#

'''
Combine the converted Snac Data

usage: snac_combine.py modelname timestep nodex nodey nodez nprocx nprocy nprocz
'''

class Combine(object):

	def __init__(self, grid):
		# data storage
		self.saved = {}
		self.saved["positions"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["velocities"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["force"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["temperature"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["plstrain"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["phaseIndex"] = [0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["irheology"] = [0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["viscosity"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["stress"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["pressure"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["strain_rate"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.struct = {"velocities":"","force":"","strain_rate":"","stress":"","pressure":"","temperature":"","plstrain":"","viscosity":"","irheoogy":"","phaseIndex":""}
		self.data = {"positions":[],"velocities":[],"strain_rate":[],"stress":[],"pressure":[],"temperature":[],"plstrain":[],"viscosity":[],"irheology":[],"phaseIndex":[]}
		self.tempExist = False
		self.apsExist = False
		self.viscExist = False
		self.irhExist = False
		return



	def readData(self, filename):
		fp = file(filename, 'r')
		lines = fp.readlines()
		m=0
		while 1:
			if lines[m].startswith("object"):
				ids = lines[m-1].split()
				keywords = lines[m].split()
				for i in range(len(keywords)):
					if keywords[i] == "items":
						items = int(keywords[i+1])

				if ids[2] == "positions":
					self.data["positions"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "velocity":
					self.data["velocities"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "strain":
					self.data["strain_rate"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "stress":
					self.data["stress"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "pressure":
					self.data["pressure"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "force":
					self.data["force"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "phaseIndex":
					self.data["phaseIndex"] = lines[m+1:m+items+1]
					m = m+items+1
				elif ids[2] == "temperature":
					self.data["temperature"] = lines[m+1:m+items+1]
					self.tempExist = True
					m = m+items+1
				elif ids[2] == "accumulated":
					self.data["plstrain"] = lines[m+1:m+items+1]
					self.apsExist = True
					m = m+items+1
				elif ids[2] == "viscosity":
					self.data["viscosity"] = lines[m+1:m+items+1]
					self.viscExist = True
					m = m+items+1
				elif ids[2] == "rheology":
					self.data["irheology"] = lines[m+1:m+items+1]
					self.irhExist = True
					m = m+items+1
				elif ids[1] == "construct":
					break
				else:
					m = m + 1
				if m >= len(lines):
					break
			else:
				if m >= len(lines):
					break
				else:
					m = m + 1
		return self.data


	def join(self, data, me, grid, cap):
		# processor geometry
		nprocx = int(cap['nprocx'])
		nprocy = int(cap['nprocy'])
		nprocz = int(cap['nprocz'])

		mylocx = me % nprocx
		mylocy = ((me - mylocx) / nprocx) % nprocy
		mylocz = (((me - mylocx) / nprocx - mylocy) / nprocy) % nprocz
		print me, nprocx,nprocy,nprocz, mylocx, mylocy, mylocz

		# mesh geometry
		nox = int(grid['nox'])
		noy = int(grid['noy'])
		noz = int(grid['noz'])
		nex = nox - 1
		ney = noy - 1
		nez = noz - 1

		mynox = 1 + (nox-1)/nprocx
		mynoy = 1 + (noy-1)/nprocy
		mynoz = 1 + (noz-1)/nprocz
		mynex = mynox - 1
		myney = mynoy - 1
		mynez = mynoz - 1

		if not len(data["positions"]) == mynox * mynoy * mynoz:
			print mynox, mynoy, mynoz, mynox * mynoy * mynoz, len(data["positions"])
			raise ValueError, "data size"
		if not len(data["stress"]) == (mynox-1) * (mynoy-1) * (mynoz-1):
			print (mynox-1),(mynoy-1),(mynoz-1), len(data["stress"])
			raise ValueError, "data size"

		mynxs = (mynox - 1) * mylocx
		mynys = (mynoy - 1) * mylocy
		mynzs = (mynoz - 1) * mylocz
		myexs = mynex * mylocx
		myeys = myney * mylocy
		myezs = mynez * mylocz

		n = 0
		for i in range(mynzs, mynzs+mynoz):
			for j in range(mynys, mynys + mynoy):
				for k in range(mynxs, mynxs + mynox):
					m = k + j * nox + i * nox * noy
					self.saved["positions"][m] = data["positions"][n]
					self.saved["velocities"][m] = data["velocities"][n]
					self.saved["force"][m] = data["force"][n]
					if self.tempExist:
						self.saved["temperature"][m] = data["temperature"][n]
					n += 1

		n = 0
		for i in range(myezs, myezs+mynez):
			for j in range(myeys, myeys + myney):
				for k in range(myexs, myexs + mynex):
					m = k + j * nex + i * nex * ney
					self.saved["strain_rate"][m] = data["strain_rate"][n]
					self.saved["stress"][m] = data["stress"][n]
                                        self.saved["pressure"][m] = data["pressure"][n]
					self.saved["phaseIndex"][m] = data["phaseIndex"][n]
					if self.apsExist:
						self.saved["plstrain"][m] = data["plstrain"][n]
					if self.viscExist:
						self.saved["viscosity"][m] = data["viscosity"][n]
					if self.irhExist:
						self.saved["irheology"][m] = data["irheology"][n]
					n += 1

		return


	def write(self, filename, grid, data, type, fp, count):
		if type == "positions":
			print >> fp, "\n# the positions array"
			print >> fp, "object %d class array type float rank 1 shape 3 items %d data follows" % (count, grid['nox']*grid['noy']*grid['noz'])
			fp.writelines(data[type])
			return count + 1
		elif type == "connections":
			print >> fp, "\n# the regular connections"
			print >> fp, "object %d class gridconnections counts %d %d %d" % (count, grid['noz'],grid['noy'],grid['nox'])
			return count + 1
		elif type == "velocities":
			print >> fp, "\n# the velocities array"
			print >> fp, "object %d class array type float rank 1 shape 3 items %d data follows" % (count, grid['nox']*grid['noy']*grid['noz'])
			fp.writelines(data["velocities"])
			self.struct[type] = '''object "velocities" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "strain_rate":
			print >> fp, "\n# the strain rate array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["strain_rate"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "strain_rate" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "stress":
			print >> fp, "\n# the stress array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["stress"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "stress" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "pressure":
			print >> fp, "\n# the pressure array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["pressure"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "pressure" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "force":
			print >> fp, "\n# the force array"
			print >> fp, "object %d class array type float rank 1 shape 3 items %d data follows" % (count, grid['nox']*grid['noy']*grid['noz'])
			fp.writelines(data["force"])
			self.struct[type] = '''object "force" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "phaseIndex":
			print >> fp, "\n# the phaseIndex array"
			print >> fp, "object %d class array type int rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["phaseIndex"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "phaseIndex" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "temperature":
			print >> fp, "\n# the temperature array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, grid['nox']*grid['noy']*grid['noz'])
			fp.writelines(data["temperature"])
			self.struct[type] = '''object "temperature" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "plstrain":
			print >> fp, "\n# the accumulated plastic strain array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["plstrain"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "plstrain" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "viscosity":
			print >> fp, "\n# the viscosity array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["viscosity"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "viscosity" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "irheology":
			print >> fp, "\n# the irheology array"
			print >> fp, "object %d class array type float rank 0 items %d data follows" % (count, (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1))
			fp.writelines(data["irheology"])
			print >> fp, 'attribute "dep" string "connections"'
			self.struct[type] = '''object "irheology" class field
component "positions" value 1
component "connections" value 2
component "data" value %d
''' % (count)
			return count + 1
		elif type == "data_structure":
			print >> fp, "\n# construct data structure"
			for member in self.struct:
				if self.struct[member] != "":
					print >> fp, "%s" % (self.struct[member])
			print >> fp, "object \"default\" class group"
			for member in self.struct:
				if self.struct[member] != "":
					print >> fp, "member \"%s\" value \"%s\"" % (member,member)
			print >> fp, "End"
			return



if __name__ == '__main__':

	import sys
	if not len(sys.argv) == 9:
		print __doc__
		sys.exit(1)

	prefix = sys.argv[1]
	step = int(sys.argv[2])

	grid = {}
	grid['nox'] = int(sys.argv[3])
	grid['noy'] = int(sys.argv[4])
	grid['noz'] = int(sys.argv[5])

	cap = {}
	cap['nprocx'] = int(sys.argv[6])
	cap['nprocy'] = int(sys.argv[7])
	cap['nprocz'] = int(sys.argv[8])

	nproc = cap['nprocx'] * cap['nprocy'] * cap['nprocz']

	cb = Combine(grid)
	for n in range(0, nproc):
		filename = 'snac.%d.%06d.dx' % (n, step)
		print 'reading', filename
		data = cb.readData(filename)
		cb.join(data, n, grid, cap)


	filename = '%s.%06d.dx' % (prefix,step)
	print 'writing', filename
	fp = open(filename, 'w')
	print >> fp, "# OpenDX DataFile Snac simulation output ASCII"
	count = 1
	count = cb.write(filename, grid, cb.saved, "positions", fp, count)
	count = cb.write(filename, grid, cb.saved, "connections", fp, count)
	count = cb.write(filename, grid, cb.saved, "velocities", fp, count)
	count = cb.write(filename, grid, cb.saved, "strain_rate", fp, count)
	count = cb.write(filename, grid, cb.saved, "stress", fp, count)
	count = cb.write(filename, grid, cb.saved, "pressure", fp, count)
	count = cb.write(filename, grid, cb.saved, "force", fp, count)
	count = cb.write(filename, grid, cb.saved, "phaseIndex", fp, count)
	if cb.tempExist:
		count = cb.write(filename, grid, cb.saved, "temperature", fp, count)
	if cb.apsExist:
		count = cb.write(filename, grid, cb.saved, "plstrain", fp, count)
	if cb.viscExist:
		count = cb.write(filename, grid, cb.saved, "viscosity", fp, count)
	if cb.irhExist:
		count = cb.write(filename, grid, cb.saved, "irheology", fp, count)
	cb.write(filename, grid, cb.saved, "data_structure", fp, count)
	fp.close()

# End of file
