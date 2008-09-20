#!/usr/bin/env python
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# <LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#



class Outlet(object):

    def __init__(self):
        self._handle = None
        return


    def send(self):
        import Snac.pyre.Exchanger as Exchanger
        Exchanger.Outlet_send(self._handle)
        return




"""
class SVTOutlet(Outlet):

    def __init__(self, source, all_variables):
        import Snac.pyre.Exchanger as Exchanger
        self._handle = Exchanger.SVTOutlet_create(source,
                                                  all_variables)
        return



"""
class TOutlet(Outlet):

    def __init__(self, source, all_variables):
        import Snac.pyre.Exchanger as Exchanger
        self._handle = Exchanger.TOutlet_create(source,
                                                all_variables)
        return

class SOutlet(Outlet):

    def __init__(self, source, all_variables):
        import Snac.pyre.Exchanger as Exchanger
        self._handle = Exchanger.SOutlet_create(source,
												all_variables)
        return

"""
class VOutlet(Outlet):

    def __init__(self, source, all_variables):
        import Snac.pyre.Exchanger as Exchanger
        self._handle = Exchanger.VOutlet_create(source,
                                                all_variables)
        return



class TractionOutlet(Outlet):


    def __init__(self, source, all_variables, mode='F'):
        import CitcomS.Exchanger as Exchanger
        self._handle = Exchanger.TractionOutlet_create(source,
                                                       all_variables,
                                                       mode)
        return

"""


# version
__id__ = "$Id: Outlet.py,v 1.4 2004/05/11 07:59:31 tan2 Exp $"

# End of file
