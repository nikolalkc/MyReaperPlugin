mywindow=nil

function guitick()
  if reaper.MRP_WindowIsClosed(mywindow) then
    --reaper.ShowConsoleMsg("closed\n")
    return
  end
  if reaper.MRP_GetWindowDirty(mywindow,0) then
    local text = reaper.MRP_GetControlText(mywindow,"Line edit 1")
    local val = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 1")
    local val2 = reaper.MRP_GetControlFloatNumber(mywindow,"Slider 2")
    reaper.ShowConsoleMsg(val.." "..val2.."\n")
    reaper.MRP_WindowSetTitle(mywindow,"Window "..text)
    reaper.MRP_SetWindowDirty(mywindow,false,0)
  end
  if reaper.MRP_GetWindowDirty(mywindow,1) then
    local w = reaper.MRP_GetWindowPosSizeValue(mywindow,2)
    local h = reaper.MRP_GetWindowPosSizeValue(mywindow,3)
    reaper.MRP_SetControlBounds(mywindow,"Slider 1",w/2,5,w/2-10,20)
    reaper.MRP_SetControlBounds(mywindow,"Slider 2",5,30,w-10,h-30)
    --reaper.ShowConsoleMsg("resized to "..w.." "..h.."\n")
    reaper.MRP_SetWindowDirty(mywindow,false,1)
  end
  reaper.defer(guitick)
end

mywindow=reaper.MRP_CreateWindow("My window")
reaper.MRP_WindowAddSlider(mywindow,"Slider 1",100)
reaper.MRP_WindowAddSlider(mywindow,"Slider 2",900)
guitick()


