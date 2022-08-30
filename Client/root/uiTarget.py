''' 1. '''
# Add to the bottom of the document
if app.ENABLE_SHIP_DEFENSE:
	class AllianceTargetBoard(ui.ThinBoard):
		class TextToolTip(ui.Window):
			def __init__(self):
				ui.Window.__init__(self, "TOP_MOST")

				textLine = ui.TextLine()
				textLine.SetParent(self)
				textLine.SetHorizontalAlignCenter()
				textLine.SetOutline()
				textLine.Show()
				self.textLine = textLine

			def __del__(self):
				ui.Window.__del__(self)

			def SetText(self, text):
				self.textLine.SetText(text)

			def OnRender(self):
				(mouseX, mouseY) = wndMgr.GetMousePosition()
				self.textLine.SetPosition(mouseX, mouseY + 30)

		def __init__(self):
			ui.ThinBoard.__init__(self)

			name = ui.TextLine()
			name.SetParent(self)
			name.SetDefaultFontName()
			name.SetOutline()
			name.Show()

			hpGauge = ui.Gauge()
			hpGauge.SetParent(self)
			hpGauge.MakeGauge(80, "red")
			hpGauge.SetPosition(10, 25)
			hpGauge.SetOverEvent(ui.__mem_func__(self.IsIn))
			hpGauge.SetOverOutEvent(ui.__mem_func__(self.IsOut))
			hpGauge.Hide()

			self.name = name
			self.hpGauge = hpGauge

			self.toolTipHP = self.TextToolTip()
			self.toolTipHP.Hide()

			self.Initialize()
			self.ResetTargetBoard()

		def __del__(self):
			ui.ThinBoard.__del__(self)

		def Initialize(self):
			self.nameLength = 0
			self.vid = 0

		def Destroy(self):
			self.name = None
			self.hpGauge = None
			self.tooltipHP = None

			self.Initialize()

		def Close(self):
			self.Initialize()
			self.tooltipHP.Hide()
			self.Hide()

		def ResetTargetBoard(self):
			self.Initialize()

			self.name.SetPosition(0, 13)
			self.name.SetHorizontalAlignCenter()
			self.name.SetWindowHorizontalAlignCenter()

			self.hpGauge.Hide()
			self.SetSize(100, 40)

		def SetTargetVID(self, vid):
			self.vid = vid

		def SetTarget(self, vid):
			self.SetTargetVID(vid)

			name = chr.GetNameByVID(vid)
			self.SetTargetName(name)

		def GetTargetVID(self):
			return self.vid

		def SetTargetName(self, name):
			self.nameLength = len(name)
			self.name.SetText(name)

		def SetHP(self, hp, hpMax):
			hp = min(hp, hpMax)
			if hp > 0:
				self.SetSize(100, self.GetHeight())

				if localeInfo.IsARABIC():
					self.name.SetPosition(self.GetWidth() - 10, 10)
				else:
					self.name.SetPosition(10, 10)

				self.name.SetWindowHorizontalAlignLeft()
				self.name.SetHorizontalAlignLeft()
				self.hpGauge.Show()
				self.UpdatePosition()

			self.hpGauge.SetPercentage(hp, hpMax)
			self.toolTipHP.SetText("%s : %d / %d" % (localeInfo.TASKBAR_HP, hp, hpMax))

		def UpdatePosition(self):
			# NOTE : y = miniMap + serverInfo Height
			self.SetPosition(wndMgr.GetScreenWidth() - self.GetWidth() - 18, 250)

		def IsOut(self):
			if self.toolTipHP:
				self.toolTipHP.Hide()

		def IsIn(self):
			if self.toolTipHP:
				self.toolTipHP.Show()

		# NOTE : Unused.
		def SetMouseEvent(self):
			pass
