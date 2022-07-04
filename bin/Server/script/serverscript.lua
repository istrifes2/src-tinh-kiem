Include("\\script\\lib\\worldlibrary.lua");
Include("\\script\\header\\npcfile.lua");
Include("\\script\\startgame\\traplib.lua");
Include("\\script\\startgame\\thon\\balanghuyen.lua");
Include("\\script\\startgame\\thon\\giangtanthon.lua");
Include("\\script\\startgame\\thon\\longmontran.lua");
Include("\\script\\startgame\\thon\\namnhactran.lua");

Include("\\script\\startgame\\thanh\\tuongduong.lua");
Include("\\script\\startgame\\thanh\\bienkinh.lua");
Include("\\script\\startgame\\thanh\\daily.lua");
Include("\\script\\startgame\\thanh\\duongchau.lua");
Include("\\script\\startgame\\thanh\\laman.lua");
Include("\\script\\startgame\\thanh\\thanhdo.lua");
Include("\\script\\startgame\\thanh\\phuongtuong.lua");

Include("\\script\\startgame\\tinhnang\\daotaytuy.lua");
Include("\\script\\startgame\\tinhnang\\bdtongkimcao.lua");
Include("\\script\\startgame\\tinhnang\\dienvotruong.lua");
Include("\\script\\startgame\\tinhnang\\vantieu.lua"); --van tieu
Include("\\script\\startgame\\khac\\satthu.lua");--boss sat thu
Include("\\script\\startgame\\khac\\othermap.lua");--add npc linh tinh
Include("\\script\\startgame\\khac\\npcnhiemvu.lua");--add npc linh tinh
Include("\\script\\startgame\\monster\\trainingmap.lua");--add quai' luyen cong

function main()
	addtrapbalang()
	addobjbalang()
	
	addtrapgiangtan()
	addobjgiangtan()
	
	addtraplongmon()
	addobjlongmon()
	
	addtrapnamnhac()
	addobjnamnhac()
	
	addtraptuongduong()
	addobjtuongduong()
	
	addtrapbienkinh()
	addobjbienkinh()
	
	addtrapdaily()
	addobjdaily()
	
	addtrapduongchau()
	addobjduongchau()
	
	addtraplaman()
	addobjlaman()
	
	addtrapthanhdo()
	addobjthanhdo()
	
	addtrapphuongtuong()
	addobjphuongtuong()
	
	traptkcaocap()
	objtkimcaocap()
	
--phan add tong hop cac loai khac

	addtrapother()
	addobjother()
	
	addtraptaytuy()
	
	if(SERVER_MODE == 1) then
		local nCurTime = tonumber(GetLocalDate("%y%m%d%H%M"))
		if(nCurTime > SERVER_OPEN) then
			addfullnpc()
		end;
	else
		addfullnpc()
	end;
end

function addfullnpc()
	addnpcbalang()
	
	addnpcgiangtan()
	
	addnpclongmon()
	
	addnpcnamnhac()
	
	addnpctuongduong()
	
	addnpcbienkinh()
	
	addnpcdaily()
	
	addnpcduongchau()
	
	addnpclaman()
	
	addnpcthanhdo()
	
	addnpcphuongtuong()
	
	npctkimcaocap()
	
--phan add tong hop cac loai khac
	addnpcsatthu()

	addnpcother()
	
	addnpctaytuy()
	
	addnpcdienvotruong()
	
	addnpcnhiemvu()
	addtrainingnpc()
	
	addnpcvantieu() -- van tieu
	addtrapvantieu() -- van tieu
end;