''' 1. '''
# Search @ def __init__
		self.uitargetBoard = None

# Add below
		if app.ENABLE_SHIP_DEFENSE:
			self.uiAllianceTargetBoard = None

''' 2. '''
# Search
	## Show & Hide
	def ShowDefaultWindows(self):

# Add above
	if app.ENABLE_SHIP_DEFENSE:
		def SetAllianceTargetBoard(self, targetBoard):
			self.uiAllianceTargetBoard = targetBoard

''' 3. '''
# Search @ def HideAllWindows
		if self.uitargetBoard:
			self.uitargetBoard.Hide()

# Add below
		if app.ENABLE_SHIP_DEFENSE:
			if self.uiAllianceTargetBoard:
				self.uiAllianceTargetBoard.Hide()
