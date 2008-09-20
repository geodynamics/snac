#!/usr/bin/env python
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# <LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#

'''
Combine the pasted Citcom Data

usage: combine.py modelname timestep nodex nodey nodez nprocx nprocy nprocz
'''

class Combine(object):

	def __init__(self, grid):
		# data storage
		self.saved = {}
		self.saved["positions"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["velocity"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["force"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["temperature"] = [0.0]*(grid['nox'] * grid['noy'] * grid['noz'])
		self.saved["plStrain"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["phase"] = [0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["irheology"] = [0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["viscosity"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["stress"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["pressure"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.saved["strainRate"] = [0.0]*((grid['nox']-1) * (grid['noy']-1) * (grid['noz']-1))
		self.struct = {"velocity":"","force":"","strainRate":"","stress":"","pressure":"","temperature":"","plStrain":"","viscosity":"","irheoogy":"","phase":""}
		self.data = {"positions":[],"velocity":[],"strainRate":[],"stress":[],"pressure":[],"temperature":[],"plStrain":[],"viscosity":[],"force":[],"phase":[]}
		self.tempExist = False
		self.apsExist = False
		self.viscExist = False
		return



	def readData(self, filename):
		fp = file(filename, 'r')
		lines = fp.readlines()
		m=0
		cellnum=0
		pointnum=0
		datanum=0
		while 1:
			if lines[m].startswith("POINTS"):
				ids = lines[m].split()
				pointnum = int(ids[1])
				self.data["positions"] = lines[m+1:m+pointnum+1]
				m = m+pointnum+1
			elif lines[m].startswith("CELL_DATA"):
				ids = lines[m].split()
				cellnum = int(ids[1])
				datanum = cellnum
				m = m+1
			elif lines[m].startswith("SCALARS"):
				ids = lines[m].split()
				for i in range(datanum):
					self.data[ids[1]] = lines[m+2].split()
				if ids[1] == "plStrain":
					self.apsExist = True
				elif ids[1] == "temperature":
					self.tempExist = True
				elif ids[1] == "viscosity":
					self.viscExist = True
				m = m+3
			elif lines[m].startswith("POINT_DATA"):
				ids = lines[m].split()
				if pointnum != int(ids[1]):
					print "coord point num != point data num!!!!!!"
					break
				datanum = pointnum
				m = m+1
			elif lines[m].startswith("VECTORS"):
				ids = lines[m].split()
				self.data[ids[1]] = lines[m+1:m+pointnum+1]
				m = m+pointnum+1
			else:
				m = m+1

			if m+1 >= len(lines):
				break

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
					self.saved["velocity"][m] = data["velocity"][n]
					self.saved["force"][m] = data["force"][n]
					if self.tempExist:
						self.saved["temperature"][m] = data["temperature"][n]+" "
					n += 1

		n = 0
		for i in range(myezs, myezs+mynez):
			for j in range(myeys, myeys + myney):
				for k in range(myexs, myexs + mynex):
					m = k + j * nex + i * nex * ney
					self.saved["strainRate"][m] = data["strainRate"][n]+" "
					self.saved["stress"][m] = data["stress"][n]+" "
					self.saved["pressure"][m] = data["pressure"][n]+" "
					self.saved["phase"][m] = data["phase"][n]+" "
					if self.apsExist:
						self.saved["plStrain"][m] = data["plStrain"][n]+" "
					if self.viscExist:
						self.saved["viscosity"][m] = data["viscosity"][n]+" "
					n += 1

		return


	def write(self, filename, grid, data, type, fp):
		if type == "positions":
			print >> fp, "POINTS %d float" % (grid['nox']*grid['noy']*grid['noz'])
			fp.writelines(data[type])
		elif type == "strainRate":
			print >> fp, "\nSCALARS strainRate float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["strainRate"])
		elif type == "stress":
			print >> fp, "\nSCALARS stress float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["stress"])
		elif type == "plStrain":
			print >> fp, "\nSCALARS plStrain float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["plStrain"])
		elif type == "pressure":
			print >> fp, "\nSCALARS pressure float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["pressure"])
		elif type == "phase":
			print >> fp, "\nSCALARS phase float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["phase"])
		elif type == "viscosity":
			print >> fp, "\nSCALARS viscosity float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["viscosity"])
		elif type == "velocity":
			print >> fp, "\nVECTORS velocity float"
			fp.writelines(data["velocity"])
		elif type == "force":
			print >> fp, "\nVECTORS force float"
			fp.writelines(data["force"])
		elif type == "temperature":
			print >> fp, "\nSCALARS temperature float"
			print >> fp, "LOOKUP_TABLE default"
			fp.writelines(data["temperature"])



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
		filename = 'snac.%d.%06d.vtk' % (n, step)
		print 'reading', filename
		data = cb.readData(filename)
		cb.join(data, n, grid, cap)


	filename = '%s.%06d.vtk' % (prefix,step)
	print 'writing', filename
	fp = open(filename, 'w')
	print >> fp, "# vtk DataFile Version 3.0"
	print >> fp, "Snac simulation combined output"
	print >> fp, "ASCII"
	print >> fp, "\n"

	print >> fp, "DATASET STRUCTURED_GRID"
	print >> fp, "DIMENSIONS %d %d %d" % (grid['nox'], grid['noy'], grid['noz'])
	cb.write(filename, grid, cb.saved, "positions", fp)
	print >> fp, "\n"
	print >> fp, "CELL_DATA  %d" % ( (grid['nox']-1)*(grid['noy']-1)*(grid['noz']-1) )
	cb.write(filename, grid, cb.saved, "strainRate", fp)

	cb.write(filename, grid, cb.saved, "stress", fp)
	cb.write(filename, grid, cb.saved, "pressure", fp)
	cb.write(filename, grid, cb.saved, "phase", fp)
	if cb.apsExist:
		cb.write(filename, grid, cb.saved, "plStrain", fp)
	if cb.viscExist:
		cb.write(filename, grid, cb.saved, "viscosity", fp)
	print >> fp, "\n"
	print >> fp, "POINT_DATA  %d" % ( grid['nox']*grid['noy']*grid['noz'] )
	cb.write(filename, grid, cb.saved, "velocity", fp)
	cb.write(filename, grid, cb.saved, "force", fp)
	if cb.tempExist:
		cb.write(filename, grid, cb.saved, "temperature", fp)

	fp.close()

# End of file
