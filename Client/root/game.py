''' 1. '''
# Search @ def __init__
		self.targetBoard = None

# Add below
		if app.ENABLE_SHIP_DEFENSE:
			self.allyTargetBoard = None

''' 2. '''
# Search @ def __init__
		self.targetBoard = uiTarget.TargetBoard()
		self.targetBoard.SetWhisperEvent(ui.__mem_func__(self.interface.OpenWhisperDialog))
		self.targetBoard.Hide()
		self.interface.SettargetBoard(self.targetBoard)

# Add below
		if app.ENABLE_SHIP_DEFENSE:
			self.allyTargetBoard = uiTarget.AllianceTargetBoard()
			self.allyTargetBoard.Hide()
			self.interface.SetAllianceTargetBoard(self.allyTargetBoard)

''' 3. '''
# Search @ def Close
		if self.targetBoard:
			self.targetBoard.Destroy()
			self.targetBoard = None

# Add below
		if app.ENABLE_SHIP_DEFENSE:
			if self.allyTargetBoard:
				self.allyTargetBoard.Destroy()
				self.allyTargetBoard = None

''' 4. '''
# Search
	def CloseTargetBoardIfDifferent(self, vid):

# Add above
	if app.ENABLE_SHIP_DEFENSE:
		def SetHPAllianceTargetBoard(self, vid, hp, hpMax):
			if self.interface.IsHideUiMode == True:
				return

			if not vid:
				self.allyTargetBoard.Close()
				return

			if vid != self.allyTargetBoard.GetTargetVID():
				self.allyTargetBoard.ResetTargetBoard()
				self.allyTargetBoard.SetTarget(vid)

			self.allyTargetBoard.SetHP(hp, hpMax)
			self.allyTargetBoard.Show()
