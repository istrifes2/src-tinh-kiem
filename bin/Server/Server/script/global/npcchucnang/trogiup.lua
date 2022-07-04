--hoangnhk
Include("\\script\\lib\\worldlibrary.lua");
Include("\\script\\header\\factionhead.lua");
Include("\\script\\header\\taskid.lua");
Include("\\script\\header\\testgame.lua");
Include("\\script\\header\\thuongtop.lua");

NEED_EMPTY = "Xin s¾p xÕp hµnh trang 6x6 « trèng"
function main(nIndex)
	local tb = {};
	local nTaskValue = GetTask(TASK_TANTHU);
	if(SERVER_MODE ~= 0) then
		if(GetNumber(1,nTaskValue,6) <= 0) then
			tinsert(tb,"NhËn hç trî t©n thñ/hotronew");
		end;
		if(GetFactionNo() >= 0 and GetNumber(1,nTaskValue,7) <= 0) then
			tinsert(tb,"NhËn th­ëng kü n¨ng/hotrokn");
		end;
	else
		if(GetNumber(1,nTaskValue,4) <= 0) then
			tinsert(tb,"Hç trî thö nghiÖm/test");
		end;
		if(GetFactionNo() >= 0 and GetNumber(1,nTaskValue,2) <= 0) then
			tinsert(tb,"Hç trî kü n¨ng/kynangtest");
		end;
		if(GetNumber(1,nTaskValue,5) <= 0) then
			tinsert(tb,"Hç trî lËp bang/bangtest");
		end;
		tinsert(tb,"NhËn trang bÞ/xanhtest");
		tinsert(tb,"NhËn ng©n l­îng/tientest");
		tinsert(tb,"Thanh lý hµnh trang/thanhly");
	end;
	if(SERVER_MODE == 1) then
		tinsert(tb,"NhËn th­ëng ®ua top/duatop");
	end;
	tinsert(tb,"KÕt thóc ®èi tho¹i/no");
	Say("LINK:<npc>",random(0,3),15277,getn(tb),tb);
end

function thanhly()
	local nT, nG = 0;
	for i = 0,5 do
		for j = 0, 9 do
		nT,nG = FindItemEx(3, i, j);
		if (nT > 0 and nG == 0) then
			RemoveItem(nT, 1)
		end
		end
	end
end

function no()
end;

function tientest()
	if(GetCash() < 1000000000) then
		Earn(10000000);
	end;
end;

function bangtest()
	local nTaskValue = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskValue,5) > 0) then
		Talk(1, "", 15278)
	return end
	if(GetCamp()==4 and GetTongFlag()==0) then
	if(GetCash()<20000000) then
	Earn(20000000-GetCash());
	end;
	AddRepute(300-GetRepute());
	AddFuYuan(500-GetFuYuan());
	SetLeadLevel(30);
	AddItemID(AddItem(0,4,195,0,0,5,0,0));
	NewWorld(2,2350,3499);
	SetFightState(1);
	SetTask(TASK_TANTHU, SetNumber(1,nTaskValue,5,1));
	else
	Talk(1,"","B¹n ch­a xuÊt s­ hoÆc ®· gia nhËp bang héi");
	end
end;

function kynangtest()
	local nTaskValue = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskValue,2) > 0) then
		Talk(1, "", "B¹n ®· nhËn hç trî nµy råi!")
	return end
	local nCurFac = GetFactionNo();
	if(nCurFac < 0 or nCurFac > 10) then
	Msg2Player("B¹n ch­a gia nhËp m«n ph¸i!")
	return end
	nCurFac = nCurFac +1;
	for i =1,getn(SKILL90_ARRAY[nCurFac]) do
		if(SKILL90_ARRAY[nCurFac][i][3] > 0) then
		AddMagic(SKILL90_ARRAY[nCurFac][i][2],20);
		else
		AddMagic(SKILL90_ARRAY[nCurFac][i][2],0);
		end
	end
	AddMagic(SKILL120AR[nCurFac],20);
	AddMagic(210,1);
	SetTask(TASK_TANTHU, SetNumber(1,nTaskValue,2,1));
	Talk(1,"","Vâ c«ng ®· ®­îc truyÒn thô, h·y thö vËn c«ng xem sao!");
end;

function test()
	local nTaskValue = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskValue,4) > 0) then
		Talk(1, "", 15278)
	return end
	if(CheckRoom(6,6) == 0) then
		Talk(1, "", 12266)
	return end
	SetTask(TASK_TANTHU, SetNumber(1,nTaskValue,4,1));
	AddItemID(AddItem(0,6,2405,0,0,5,0,0))--mabai
	AddItemID(AddItem(0,5,1,0,0,0,0,0))--thp
	AddItemID(AddItem(0,5,2,0,0,0,0,0))--tdpvh
	for i=158,166 do
		AddItemID(AddItem(2,0,i,0,10,0,10,0));
	end;
	SetExtPoint(50000);
	Earn(10000000);
	SetLevel(160);
end;

function duatop()
	if(GetLevel() < 120) then
		Talk(1, "", format(NOT_LEVEL,120))
	return end
	local nTaskValue = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskValue,3) > 0) then
		Talk(1, "", 15278)
	return end
	local nDone = GetDataInt(DATA_DUATOP);
	if(nDone >= 10) then
		Talk(1, "", 10288)
	return end
	if(CheckRoom(6,9) == 0) then
		Talk(1, "", 12266)
	return end
	nDone = nDone+1
	SetData(DATA_DUATOP, nDone);
	SetTask(TASK_TANTHU, SetNumber(1,nTaskValue,3,1));
	Talk(1, "", 10289)
	Msg2SubWorld(format("<color=green>%s<color>nhËn th­ëng h¹ng <color=yellow>%d",GetName(), nDone));
	nhanvatpham(nDone);
	SaveDataFile();
end;

function nhanvatpham(nNo)
	if(nNo > 0 and nNo < 11) then
	end
end;

NEED_6X5 = "Hµnh trang ph¶i cã 6x7 « trèng (7 hµng)"

function hotronew()
	local nTaskVal = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskVal,6) > 0) then
	Talk(1, "", 15278)
	return end
	if(CheckRoom(6,7) == 0) then
	Talk(1, "", NEED_6X7);
	return end
	SetTask(TASK_TANTHU, SetNumber(1,nTaskVal,6,1));
	Earn(5000)
	AddTimeItem(LockItem(AddItemID(AddItem(0,5,1,0,0,0,0,0)),-2),15*24*60*60)--tdpvh
	--AddTimeItem(LockItem(AddItemID(AddItem(0,5,2,0,0,0,0,0)),-2),15*24*60*60)--thp
	LockItem(AddItemID(AddItem(2,0,176,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,177,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,181,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,182,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,183,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,184,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,178,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,179,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(2,0,180,0,0,0,5,0)),1,0,1);
	LockItem(AddItemID(AddItem(0,0,10,random(1,3),7,0,0,0)),1,0,0);
	for i=1,5 do
		LockItem(AddItemID(AddItem(0,6,1182,0,0,5,0,0)),1,0,1);
	end;
end;

function hotrokn()
	local nCurFac = GetFactionNo();
	if(nCurFac < 0 or nCurFac > 10) then
	Msg2Player("B¹n ch­a gia nhËp m«n ph¸i!")
	return end
	local nTaskVal = GetTask(TASK_TANTHU);
	if(GetNumber(1,nTaskVal,7) > 0) then
	Talk(1, "", 15278)
	return end
	SetTask(TASK_TANTHU, SetNumber(1,nTaskVal,7,1));
	show_kynang90(nCurFac+1)
end;
