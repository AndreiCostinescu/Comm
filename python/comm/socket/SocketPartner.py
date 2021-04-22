from typing import Optional, Tuple


class SocketPartner:
    def __init__(self, overwritePartner: bool, initializePartner: bool):
        self.partner = ("0.0.0.0", 0) if initializePartner else None  # type: Optional[Tuple[str, int]]
        self.overwrite = overwritePartner
        self.isCopy = False

    @staticmethod
    def SocketPartner(partner: Tuple[str, int], overwritePartner: bool):
        new = SocketPartner(overwritePartner, False)
        new.partner = partner
        return new

    def copy(self):
        copy = SocketPartner.SocketPartner(self.partner, self.overwrite)
        copy.isCopy = True
        return copy

    def isInitialized(self):
        return self.partner is not None

    def setOverwrite(self, _overwrite: bool):
        self.overwrite = _overwrite

    def setPartner(self, _partner: Tuple[str, int]):
        self.isCopy = False
        self.partner = _partner

    def getOverwrite(self):
        return self.overwrite

    def getPartner(self):
        return self.partner

    def getPartnerString(self):
        return self.getStringAddress()

    def getIP(self):
        return self.partner[0] if self.partner is not None else ""

    def getPort(self):
        return self.partner[1] if self.partner is not None else 0

    def getStringAddress(self):
        return self.getIP() + ":" + str(self.getPort())

    def cleanup(self):
        self.partner = None
